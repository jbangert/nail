#include "nailtool.h"

#include <list>
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}
#define DEBUG_OUT

//static const std::string parser_template(_binary_parser_template_c_start,_binary_parser_template_c_end - _binary_parser_template_c_start);
class CDataModel{
  std::ostream &out;

  void emit_array(const parser &inner, const std::string name = ""){

    out << "struct "<< name <<"{\n";
    emit_type(inner);
    out << "*elem;\n size_t count;\n";
    out << "}" ;

  }
  void emit_type( const parser &outer,const std::string name =""){
    const parserinner &p = outer.PR;
    switch(p.N_type){
    case INT:
      out << int_type(p.INT.parser);
      break;
    case STRUCT:
      out<<"struct " << name << "{\n";
      FOREACH(field,p.STRUCT){
        if(field->N_type != FIELD)
          continue;
        emit_type(*field->FIELD.parser);
        out << " " << mk_str(field->FIELD.name) << ";\n";
      }
      out << "}" << std::endl;
      break;
    case WRAP:
      emit_type(*p.WRAP.parser);
      break;
    case CHOICE:{
      out << "struct " << name<< "{\n enum  {";
      int idx=0;
      FOREACH(option, p.CHOICE){
        if(idx++ >0) 
          out << ',';
        out << mk_str(option->tag);
      }
      out << "} N_type; \n";
      out << "union {\n";
      FOREACH(option, p.CHOICE){
        emit_type(*option->parser);
        out << " "<<  mk_str(option->tag) << ";\n";
      }
      out<< "};\n}";
    }
      break;
  
    case ARRAY:
      switch(p.ARRAY.N_type){
      case MANY:
      case MANYONE:
        emit_array(*p.ARRAY.MANY,name);
        break;
      case SEPBY:
      case SEPBYONE:
        emit_array(*p.ARRAY.SEPBY.inner,name);
        break;
      }
      break;
    case FIXEDARRAY:
      emit_type(*p.FIXEDARRAY.inner);
      out << "["<< intconstant_value(p.FIXEDARRAY.length) << "]";
      break;
    case OPTIONAL:
      emit_type(*p.OPTIONAL);
      out << "*";
      break;
    case LENGTH:
      emit_array(*p.LENGTH.parser,name);
      break;

    case UNION:
      {
        std::string first;
        int idx=0;
        FOREACH(elem, p.UNION){
          //  std::stringstream tmp;
          //emit_type(tmp,**elem);
          if(idx++==0)
            emit_type(**elem);
          //            first = tmp.str();
          else{
            //TODO: Perform in-depth checking
            //         if(first!= tmp.str()){
            //        throw std::runtime_error(boost::str(boost::format("UNION elements do not induce the same type\n %s \n %s") % tmp.str() % first)); //Be more
            //descriptive
            //    }
          }
        }
        out << first;
      }
      break;
    case REF:
      out << mk_str(p.REF.name)<<"* "; // TODO: Make ref more powerful, featuring ALIAS
      break;
    case NAME:
      out << mk_str(p.NAME.name);
      break;
    case APPLY:
      emit_type(*p.APPLY.inner); 
      break;
    }
  }
  void emit_type_definition( const parser &p,const std::string name)
  {
    switch(p.PR.N_type){
    case STRUCT:
    case ARRAY:
    case CHOICE:
      emit_type(p,name);
      out << ";" << std::endl;
      break;
    case WRAP:
      emit_type_definition(*p.PR.WRAP.parser,name);
      break;
    default:
      break; //Other definitions don't need a type
    }
  }
  void emit_type( const definition &def){
    if(def.N_type != PARSER)
      return;
    std::string name(mk_str(def.PARSER.name));
    emit_type_definition(def.PARSER.definition,name);
  }
public:
  void emit_parser( grammar *grammar){
    try{
      assert(out.good());
      out << "#include <stdint.h>\n #include <string.h>\n#include <assert.h>\n";
      //Emit forward declarations
      FOREACH(definition,*grammar){
        if(definition->N_type!=PARSER)
          continue;
        std::string name(mk_str(definition->PARSER.name));
        std::string post; 
        std::string typedef_t( typedef_type(definition->PARSER.definition,name,&post));
        out << "typedef "<<typedef_t<< " " << name << post <<";" << std::endl;
      }
      FOREACH(definition, *grammar){
        emit_type(*definition);
      }
      out<< std::endl;
    } catch(std::exception &e){
      std::cerr << "Exception while generating parser" << e.what()<<std::endl;
      exit(-1);
    }
  }

  CDataModel(std::ostream *os) : out(*os){}
  
};

std::string intconstant_value(const intconstant &val){
  switch(val.N_type){
  case ASCII:
    if(val.ASCII.N_type == ESCAPE)
      return boost::str(boost::format("'\\%c'") % val.ASCII.ESCAPE);
    else
      return boost::str(boost::format("'%c'") % val.ASCII.DIRECT);
    break;
  case HEX:
    return std::string("0x") + mk_str(val.HEX);
  case NUMBER:
    return mk_str(val.NUMBER);
  }
}

static std::string int_expr(unsigned int width){
  if(width>=64){
    throw std::runtime_error("More than 64 bits in integer");
  }
  //TODO: x
  // TODO: Make big endian. Flip n bytes
  return boost::str(boost::format("read_unsigned_bits(stream,%d)") % width);
}
class CAction{
  std::ostream &out;
  int num_iters;
  std::string arena; // Which arena to allocate from (local variable name)
  void action_constparser(){
#ifdef DEBUG_OUT
    out << "fprintf(stderr,\"%d = const %d\\n\",tr-trace_begin, *tr);\n";
#endif
    out << "off  = *tr;\n"
        << "tr++;\n";
  }
public:
  void action (const parserinner &p, Expr &lval){
    switch(p.N_type){
    case INT:{
      int width = boost::lexical_cast<int>(mk_str(p.INT.parser.UNSIGNED));
      out << lval << "=" << int_expr(width) << ";\n";
      out<< "off += " << width<<";\n";
    }
      break;
    case STRUCT:
      FOREACH(field, p.STRUCT){
        switch(field->N_type){
        case CONSTANT:
          action_constparser();
          break;
        case FIELD:{
          ValExpr fieldname(mk_str(field->FIELD.name),&lval);
          action(field->FIELD.parser->PR,fieldname);
          break;
        }
        case DEPENDENCY:{
          //TODO: Be more efficient
          action_constparser();

        }
          break;
        case TRANSFORM:
          break;
        }
      }
      break;
    case WRAP:
      if(p.WRAP.constbefore){
        FOREACH(c,*p.WRAP.constbefore){
          action_constparser();
        }
      }
      action(p.WRAP.parser->PR,lval);
      if(p.WRAP.constafter){
        FOREACH(c,*p.WRAP.constafter){
          action_constparser();
        }
      }      
      break;
    case CHOICE:
      {
#ifdef DEBUG_OUT
        out << "fprintf(stderr,\"%d = choice %d %d\\n\",tr-trace_begin, tr[0], tr[1]);";
#endif

        out << "switch(*(tr++)){\n";
        FOREACH(c, p.CHOICE){
          out << "case " << mk_str(c->tag) << ":\n";
          out << "tr = trace_begin + *tr;\n";
          out << ValExpr("N_type", &lval) << "= "<< mk_str(c->tag) <<";\n";
          ValExpr expr(mk_str(c->tag),&lval);
          action(c->parser->PR, expr );
          out << "break;\n";
        }
        out << "default: assert(\"BUG\");";
        out << "}";
      }      
      break;
    case UNION:
      {
#ifdef DEBUG_OUT
        out << "fprintf(stderr,\"%d = choice %d %d\\n\",tr-trace_begin, tr[0], tr[1]);\n";
#endif
        int nr_option = 1;
        out << "switch(*(tr++)){\n";
        FOREACH(c, p.UNION){
          out << "case " << nr_option++ << ":\n";
          out << "tr = trace_begin + *tr;\n";
          action((*c)->PR, lval);
          out << "break;\n";
        }
        out << "default: assert(!\"Error\"); exit(-1);";
        out << "}";
      }
      break;
    case APPLY:
      {
        out << "{/*Apply*/ "
            <<"NailStream original_stream = str_current;\n #error \" not implemented \"";
        out << "str_current = n_tr_read_stream(n_trace);";
        action(p.APPLY.inner->PR, lval);
        out << "str_current = original_stream;";
        out <<"}\n";
      }
      break;
    case LENGTH:
    case ARRAY:
      {
#ifdef DEBUG_OUT
        out << "fprintf(stderr,\"%d = many %d %d\\n\",tr-trace_begin, tr[0], tr[1]);\n";
#endif
        const parser *i;
        ValExpr count("count", &lval);
        ValExpr data("elem", &lval);
        if(p.N_type == ARRAY){
          switch(p.ARRAY.N_type){
          case MANYONE:
            i = p.ARRAY.MANYONE;break;
          case MANY:
            i = p.ARRAY.MANY; break;
          case SEPBY:
            i = p.ARRAY.SEPBY.inner; break;
          case SEPBYONE:
            i= p.ARRAY.SEPBYONE.inner; break;
          }
        }
        else{ /* LENGTH*/
          i = p.LENGTH.parser;
        }
          
        out << "{ /*ARRAY*/ \n pos save = 0;";
        out << count << "=" << "*(tr++);\n"
            << "save = *(tr++);\n";
        out <<data << "= " << "n_malloc("<<arena<<"," << count << "* sizeof(*"<<data<<"));\n"
            << "if(!"<< data<< "){return 0;}\n";

        std::string iter = boost::str(boost::format("i%d") % num_iters++);
        out << "for(pos "<<iter<<"=0;"<<iter<<"<"<<count<<";"<<iter<<"++){";
        if(p.N_type == ARRAY && (p.ARRAY.N_type == SEPBY || p.ARRAY.N_type == SEPBYONE)){
          out<< "if("<<iter<<">0){";
          action_constparser();
          out << "}";
        }
        ValExpr iexpr(iter);
        ArrayElemExpr elem(&data,&iexpr);
        action(i->PR,elem);
        out << "}\n";
        out << "tr = trace_begin + save;\n}";
      }
      break;
    case FIXEDARRAY:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      ValExpr iexpr(iter);
      ArrayElemExpr elem(&lval, &iexpr);
      out << "for(pos "<< iter<< "=0;"<<iter<<"<"<<intconstant_value(p.FIXEDARRAY.length) << ";"<<iter<<"++){";
      action(p.FIXEDARRAY.inner->PR,elem);
      out << "}";
    }
      break;
    case OPTIONAL:
      {
        out<< "if(*tr<0) /*OPTIONAL*/ {\n"
           << "tr++;\n"
           << lval << "= "<< "n_malloc("<<arena<<",sizeof(*"<<lval<<"));\n";
        out << "if(!"<<lval<<") return -1;\n";
        DerefExpr deref(lval);
        action(p.OPTIONAL->PR,deref);
        out << "}\n"
            << "else{tr = trace_begin + *tr;\n "<< lval <<"= NULL;}";
      }
      break;
    case REF:
      {
        out << lval << "= n_malloc("<<arena<<",sizeof(*"<<lval<<"));\n"
            << "if(!"<<lval<<"){return -1;}";
        out << "off = bind_"<<mk_str(p.REF.name)<< "("<<arena<<"," << lval << ", data,off,&tr,trace_begin);"
            << "if(parser_fail(off)){return -1;}\n";
        break;
      }
    case NAME:
      {
        out << "off = bind_"<<mk_str(p.NAME.name)<< "("<<arena<<",&" << lval << ", data,off,&tr,trace_begin);"
            << "if(parser_fail(off)){return -1;}";
        break;
      }
    }
  }
public:
  //Refactor this. make emit_action static, allocation its own CAction to build stuff
  CAction(std::ostream *os,std::string _arena): out(*os), num_iters(1),arena(_arena){}
 
  void emit_action(const grammar &grammar){
    FOREACH(def, grammar){
      if(def->N_type == PARSER){ 
        std::string name= mk_str(def->PARSER.name);
        out << "static pos bind_"<< name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace,  pos * trace_begin);";
      }
    }
    FOREACH(def, grammar){
      if(def->N_type == PARSER){
        std::string name= mk_str(def->PARSER.name);
        out << "static pos bind_"<<name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace ,  pos * trace_begin){\n";
        out << " pos *tr = *trace;";
        //  out << name << "*ret = malloc(sizeof("<<name<<")); if(!ret) return -1;";
        ValExpr outexpr("out",NULL,1);
        action(def->PARSER.definition.PR,outexpr);
        out << "*trace = tr;";
        out<< "return off;}";
        out << name << "*parse_" << name << "(NailArena *arena, const uint8_t *data, size_t size){\n"
            << "NailStream stream = {.data = data, .off= 0, .size = size*8};\n"
            <<"n_trace trace;\n"
            <<"pos *tr_ptr;\n pos pos;\n"
            << name <<"* retval;\n"
            << "n_trace_init(&trace,4096,4096);\n"
            << "tr_ptr = trace.trace;"
            << "if(size*8 != peg_"<<name<<"(&trace,&stream)) return NULL;"
            << "retval = n_malloc(arena,sizeof(*retval));\n"
            <<"if(!retval) return NULL;\n"
            << "stream.off = 0;\n"
            << "if(bind_"<<name<<"(arena,retval,stream,&tr_ptr,trace.trace) < 0) return NULL;\n"
            << "n_trace_release(&trace);\n"
            <<"return retval;"
            <<"}";
      }
      out << std::endl;
    }
  }
};

class CPrimitiveParser{
  std::ostream &out;
  int nr_choice,nr_option;
  int nr_many;
  int nr_const;
  int nr_dep;
  int num_iters;
  int bit_offset = 0; // < 0 = unknown alignment, 0 == byte aligned, 1 = byte + 1, etc 
  CAction dependency_action;
  /*Parse pass emits a light-weight trace of data structure*/
  //TODO: Keep track of bit alignment
  void stream_advance(unsigned int width){
    out << "stream_advance(str_current,"<<width<<");\n";
  }
  void check_int(unsigned int width, const std::string &fail){
    out << "if(!stream_check(str_current,"<<width<<")) {"<<fail<<"}\n";
  }
  void constraint(std::string val,  intconstraint &c){
    switch(c.N_type){
    case RANGE:
      out <<val<< ">"<<intconstant_value(*c.RANGE.max) << "|| "<< val <<" < "<<intconstant_value(*c.RANGE.min);
      break;
    case SET:
      {
        int first = 0;
        FOREACH(allowed,c.SET){
          if(first++ != 0)
            out << " && ";
          out << val << " != "<<intconstant_value(*allowed);
        }
      }
      break;
    case NEGATE:
      out << "!(";
      constraint(val,*c.NEGATE);
      out << ")";
      break;
    }
  }
  
  void peg(const constrainedint &c, const std::string &fail){
    int width = boost::lexical_cast<int>(mk_str(c.parser.UNSIGNED));
    check_int(width,fail);
    if(c.constraint != NULL){
      out << "{\n uint64_t val = "<< int_expr(width) << ";\n";
      out << "if(";
      constraint(std::string("val"),*c.constraint);
      out << "){"<<fail<<"}\n";
      out << "}\n";
    }
    stream_advance(width);
  }
  void peg_constint(int width, const std::string value,const std::string &fail){
    check_int(width,fail);
    out << "if( "<< int_expr(width) << "!= "<<value<<"){"<<fail<<"}";
    stream_advance(width);
  }
  void peg_const(const constarray &c, const std::string &fail){
    switch(c.value.N_type){
    case STRING:
      assert(mk_str(c.parser.UNSIGNED) == "8");
      FOREACH(ch, c.value.STRING){
        peg_constint(8,(boost::format("'%c'") % ch).str(),fail);
      }
      break;
    case VALUES:{
      int width = boost::lexical_cast<int>(mk_str(c.parser.UNSIGNED));
      FOREACH(v,c.value.VALUES){
        peg_constint(width, intconstant_value(*v),fail);
      }
      break;
    }
    }
  }

  void peg_const(const constparser &c, const std::string &fail){
    switch(c.N_type){
    case CARRAY:
      peg_const(c.CARRAY,fail);
      break;
    case CREPEAT:{
      std::string retrylabel = (boost::format("constmany_%d_repeat") % nr_const++).str();
      std::string faillabel = (boost::format("constmany_%d_end") % nr_const++).str();
      out << retrylabel << ":\n";
      peg_const(*c.CREPEAT,std::string("goto ")+faillabel+";");
      out << "goto " << retrylabel << ";\n";
      out << faillabel << ":\n";
      break;
    }
    case CINT:{
      int width = boost::lexical_cast<int>(mk_str(c.CINT.parser.UNSIGNED));
      peg_constint(width,intconstant_value(c.CINT.value),fail);
    }
      break;
    case CREF:
      out << "{\n"
          <<"pos ext = peg_" << mk_str(c.CREF) << "(data,off,max);\n"
          <<"if(ext < 0){" << fail << "}\n"
          << "off = ext;\n"
          << "}\n";
      break;
    case CSTRUCT:
      FOREACH(field,c.CSTRUCT){
        peg_const(*field,fail);
      }
      break;
    case CUNION:{
      out << "{\n"
          <<"backtrack back = stream_memo(str_current);";
      int thischoice = nr_choice++;
      std::string succ_label = boost::str(boost::format("cunion_%d_succ") % thischoice);
      int n_option = 1;
      FOREACH(option,c.CUNION){
        std::string fallthrough_label = boost::str(boost::format("cunion_%d_%d") % thischoice % n_option++);
        std::string failopt = boost::str(boost::format("goto %s;") % fallthrough_label);
        peg_const(*option,failopt);
        out << "goto " << succ_label << ";\n";
        out << fallthrough_label << ":\n";
        out << "stream_backtrack(str_current,back);\n";
      }
      out << fail << "\n";
      out << succ_label << ":;\n";
      out << "}\n";
    }
      
      break;
    }

  }
  void peg(const constparser &c, const std::string &fail){
    peg_const(c,fail);
    out << "if(parser_fail(n_tr_const(trace,off))){"<<fail<<"}\n";
  }
  struct namedparser{
    const parser *p;
    std::string name;
    namedparser(std::string _name, const parser *_p): p(_p), name(_name){}
  };
  typedef  std::list<namedparser> parserlist;
  void peg_choice(const parserlist &list, const std::string &fail, const Scope &scope){
    int this_choice = nr_choice++;
  
    out << "{backtrack back = stream_memo(str_current);\n"
        << "pos choice_begin = n_tr_begin_choice(trace); \n"
        << "pos choice;\n"
        << "if(parser_fail(choice_begin)) {" << fail <<"}\n";
    std::string success_label = (boost::format("choice_%d_succ") % this_choice).str();
    
    BOOST_FOREACH(const namedparser p, list){
      std::string fallthrough_memo  =  (boost::format("choice_%u_%s_out") % this_choice %p.name).str();
      std::string fallthrough_goto = boost::str(boost::format("goto %s;") % fallthrough_memo);
      out << "choice = n_tr_memo_choice(trace);\n";
      peg(*p.p,fallthrough_goto,scope);
      out << "n_tr_pick_choice(trace,choice_begin,"<<p.name<<",choice);";
      out << "goto " << success_label<< ";\n";
      out << fallthrough_memo << ":\n";
      out << "stream_backtrack(str_current, back);\n";

    }
    out << fail << "\n";
    out << success_label <<": ;\n";
    out << "}";
  }
  void peg_repeat(const parser &p, const std::string &fail, size_t min, const constparser *separator, const Scope &scope){
    int this_many = nr_many++;
    std::string gotofail=(boost::format("goto fail_repeat_%d;") % this_many).str();
    out << "{\n";
    out << "pos many = n_tr_memo_many(trace);\n"
        << "pos count = 0;\n"
        << "succ_repeat_" << this_many << ":\n";
    if(separator != NULL){
      out << "if(count>0){\n";
      peg(*separator,gotofail);
      out << "}\n";
    }
    peg(p, gotofail,scope);
    out << "count++;\n";
    out << " goto succ_repeat_"<< this_many << ";\n";
    out << "fail_repeat_" << this_many << ":\n";
    out << "n_tr_write_many(trace,many,count);\n" ;
    if(min>0){
      out << "if(count < "<<min<<"){"<< fail << "}\n";
    }
    out << "}\n";
    
  }
  void peg(const arrayparser &array, const std::string &fail, const Scope &scope){
    switch(array.N_type){
    case MANYONE:
      peg_repeat(*array.MANYONE,fail, 1,NULL,scope);
      break;
    case MANY:
      peg_repeat(*array.MANY,fail, 0,NULL,scope);
      break;
    case SEPBYONE:
      peg_repeat(*array.SEPBYONE.inner,fail, 1,&array.SEPBYONE.separator,scope);
      break;
    case SEPBY:
      peg_repeat(*array.SEPBY.inner,fail, 0, &array.SEPBY.separator,scope);
      break;
    }
  }
  void peg_optional(const parser &p, const std::string &fail,const Scope &scope){
    int this_many = nr_many++;
    out << "{\n";
    out << "pos many = n_tr_memo_optional(trace);\n";
    peg(p, (boost::format("goto fail_optional_%d;") % this_many).str(), scope);
    out << "n_tr_optional_succeed(trace,many); goto succ_optional_"<< this_many << ";\n";
    out << "fail_optional_" << this_many << ":\n";
    out << "n_tr_optional_fail(trace,many);\n"
        << "succ_optional_" << this_many << ":;\n";
    out << "}";
  }
  void peg(const parser &parser, const std::string &fail,const Scope &scope ){
    switch(parser.PR.N_type){
    case INT:
      peg(parser.PR.INT,fail);
      break;
    case STRUCT:{
      Scope newscope(scope);
      FOREACH(field,parser.PR.STRUCT){
        switch(field->N_type){
        case CONSTANT:
          peg(field->CONSTANT,fail);
          break;
        case FIELD:
          peg(*field->FIELD.parser,fail,newscope);
          break;
        case DEPENDENCY:
          {
            //TODO: Use BIND/TYPE/ETC
            int this_dep = ++nr_dep;
            std::string depfail = (boost::format("goto faildep_%d;") % this_dep).str();
            std::string type_suffix;
            std::string type = typedef_type(*field->DEPENDENCY.parser,"", &type_suffix);
            //            int width = boost::lexical_cast<int>(mk_str(field->DEPENDENCY.parser.parser.UNSIGNED));
            std::string name(mk_str(field->DEPENDENCY.name));
            out << "pos trace_" << name << " = n_trace_getpos(trace);\n";
            assert(type_suffix == "");

            ValExpr lval(std::string ("dep_") + name);
            peg(*field->DEPENDENCY.parser, depfail, scope);
            dependency_action.action(field->DEPENDENCY.parser->PR,lval); // TODO: Make action use a different arena
            newscope.add_dependency_definition(name,type);
            out << "n_tr_setpos(trace,trace_"<<name<<");\n";
            out << "n_tr_const(trace,off);\n";
          }
          break;
          
        case TRANSFORM:
          {
            std::stringstream header; //TODO: Actually put this into a header
            std::stringstream body;
            assert(!"Implement");
            FOREACH(stream, field->TRANSFORM.left){
              body << "NailStream str_" << mk_str(*stream) <<";\n";
              // str_current cannot appear on the left
            }
            body << mk_str(field->TRANSFORM.cfunction) << "_parse(arena";
            assert("!Check parameters");
            FOREACH(stream, field->TRANSFORM.left){
              body << ", &str_"  << mk_str(*stream);
            }      
            header << "extern int " << mk_str(field->TRANSFORM.cfunction) << "_parse(NailArena *tmp,";
            FOREACH(param, field->TRANSFORM.right){
              switch(param->N_type){
              case P_DEPENDENCY:{
                std::string name (mk_str(param->P_DEPENDENCY));
                //                if(newscope.dependency_type(name))
                //we need to emit a type for this function!
                body << "," << newscope.dependency_ptr(name); 
                header << "," << newscope.dependency_type(name) << "* " << name;
              }
                break;
              case P_STREAM:
                body << "," <<  newscope.stream_ptr(mk_str(param->P_STREAM));  
                header << ",NailStream *" << mk_str(param->P_STREAM);
                break;
              }
            }
            body << ");";
            header << (");");
            FOREACH(stream, field->TRANSFORM.left){
              newscope.add_stream_definition(mk_str(*stream));
            }
            out << header << body; //TODO: put in separate files
          }
          break;
        }

      }
    }
      break;
    case WRAP:
      if(parser.PR.WRAP.constbefore){
        FOREACH(c,*parser.PR.WRAP.constbefore){
          peg(*c,fail);
        }
      }
      peg(*parser.PR.WRAP.parser,fail,scope);
      if(parser.PR.WRAP.constafter){
        FOREACH(c,*parser.PR.WRAP.constafter){
          peg(*c,fail);
        }
      }
      break;
    case CHOICE:
      {
        parserlist l;
        FOREACH(c, parser.PR.CHOICE){
          l.push_back(namedparser(mk_str(c->tag),c->parser));
        }
        peg_choice(l,fail,scope);
      }
      break;
    case UNION:
      {
        parserlist l;
        int i = 0;
        FOREACH(c, parser.PR.UNION){
          i++;
          l.push_back(namedparser(boost::lexical_cast<std::string>(i),*c));
        }
        peg_choice(l,fail,scope);
      }
    case ARRAY:
      peg(parser.PR.ARRAY,fail,scope); 
      break;
    case FIXEDARRAY:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      
      out << "/*FIXEDARRAY*/ \n"
          << "for(pos "<<iter<<"=0;"<<iter<<"<"<<intconstant_value(parser.PR.FIXEDARRAY.length)<<";"<<iter<<"++){\n";
      peg(*parser.PR.FIXEDARRAY.inner,fail,scope);
      out << "}\n";
    }
      break;
    case LENGTH:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      out << "{/*LENGTH*/"
          << "pos many = n_tr_memo_many(trace);\n"
          << "pos count= "<< mk_str(parser.PR.LENGTH.length)<<";\n"
          << "pos "<<iter<<"=0;";
      out<< "for( "<<iter<<"=0;"<<iter<<"<count;"<<iter<<"++){";
      peg(*parser.PR.LENGTH.parser,fail,scope);
      out << "}";
      out << "n_tr_write_many(trace,many,count);\n";
      out << "}/*/LENGTH*/";
      break;
    }
    case APPLY: {
      //TODO: Record in trace - specify NailStream in the trace?
      int this_many = nr_many++;
      //TODO: Make sure each stream is only bound once!
      out << "{/*APPLY*/";
      out << "NailStream orig_str = str_current;\n"
          << "str_current = "<< mk_str(parser.PR.APPLY.stream) << ";\n"
          << "n_tr_stream(n_trace, str_current)\n";
      peg(*parser.PR.APPLY.inner,(boost::format("goto fail_apply_%d") % this_many).str(), scope);
      out << "goto succ_apply_" << this_many << "\n";
      out << "fail_apply_" << this_many << ":\n";
      out << "str_current = orig_str; \n" 
          << fail << "\n";
      out << "succ_apply_" << this_many << ":\n"
          << "str_current = orig_str;\n";
      out << "}";        
    }
    case OPTIONAL:
      peg_optional(*parser.PR.OPTIONAL,fail,scope);
      break;
      //TODO: Do caching here
    case NAME: // Fallthrough intentional, and kludgy
    case REF:
      //TODO: Deal with parameters here
      out << "i = peg_" << mk_str(parser.PR.REF.name)<< "(trace,data,off,max);";
      out << "if(parser_fail(i)){" << fail << "}\n"
          << "else{off=i;}\n";
      break;
    }
  }
public:
  CPrimitiveParser(std::ostream *os) : out(*os), nr_choice(1),nr_many(0),nr_const(0),nr_dep(0),num_iters(1), dependency_action(os,"tmp_arena"){}
  void peg(const definition &def) {
    if(def.N_type == PARSER){
      Scope scope;
      std::string name = mk_str(def.PARSER.name);
      out <<"static pos peg_" << name <<"(n_trace *trace, NailStream *str_current){\n";
      out << "pos i;\n"; //Used in name and ref as temp variables
      peg(def.PARSER.definition, "goto fail;",scope);
      out << "return off;\n"
          << "fail:\n return -1;\n";
      out << "}\n";
    }
    else if(def.N_type == CONST){
      std::string name = mk_str(def.CONST.name);
      out << "static pos peg_" << name << "(NailStream *str_current){\n";
      peg_const(def.CONST.definition, "goto fail;");
      out << "return off;\n"
          << "fail: return -1;\n"
          << "}";
    }
  }
  void emit_parser(const grammar &gram){
    FOREACH(def, gram){
      //Declaration
      switch(def->N_type){
      case PARSER:
        out << "static pos peg_" << mk_str(def->PARSER.name) <<"(n_trace *trace,NailStream *str_current);\n";
        break;
      case CONST:
        out << "static pos peg_" << mk_str(def->CONST.name) <<"(NailStream *str_current);\n";
        break;
      }
    }
    FOREACH(def, gram){
      peg(*def);
    }
    out << std::endl;
  }
};

void emit_parser(std::ostream *out, grammar *grammar){

  CPrimitiveParser p(out);
  CAction a(out,"arena");
  *out << std::string(parser_template_start,parser_template_end - parser_template_start);
  p.emit_parser(*grammar);
  a.emit_action(*grammar);
}
void emit_header(std::ostream *out, grammar *grammar){
  CDataModel data(out);
  data.emit_parser(grammar);
  *out  <<  "struct NailArenaPool;"
    "typedef struct NailArena_{"
    "     struct NailArenaPool *current;"
    "    size_t blocksize;"
    " } NailArena ;\n"
        << "int NailArena_init(NailArena *arena,size_t blocksize);\n"
        << "int NailArena_release(NailArena *arena);\n";
    
  FOREACH(def, *grammar){
    if(def->N_type == PARSER){
      std::string name= mk_str(def->PARSER.name);
      *out << name << "*parse_" << name << "(NailArena *arena, const uint8_t *data, size_t size);\n";
    }
  }

}


