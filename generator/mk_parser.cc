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
    const parserinner &p = outer.pr;
    switch(p.N_type){
    case INTEGER:
      out << int_type(p.integer.parser);
      break;
    case STRUCTURE:
      out<<"struct " << name << "{\n";
      FOREACH(field,p.structure){
        if(field->N_type != FIELD)
          continue;
        emit_type(*field->field.parser);
        out << " " << mk_str(field->field.name) << ";\n";
      }
      out << "}" << std::endl;
      break;
    case WRAP:
      emit_type(*p.wrap.parser);
      break;
    case CHOICE:{
      out << "struct " << name<< "{\n enum  {";
      int idx=0;
      FOREACH(option, p.choice){
        if(idx++ >0) 
          out << ',';
        out << mk_str(option->tag);
      }
      out << "} N_type; \n";
      out << "union {\n";
      FOREACH(option, p.choice){
        emit_type(*option->parser);
        out << " "<<  mk_str(option->tag) << ";\n";
      }
      out<< "};\n}";
    }
      break;
  
    case ARRAY:
      switch(p.array.N_type){
      case MANY:
      case MANYONE:
        emit_array(*p.array.many,name);
        break;
      case SEPBY:
      case SEPBYONE:
        emit_array(*p.array.sepby.inner,name);
        break;
      }
      break;
    case FIXEDARRAY:
      emit_type(*p.fixedarray.inner);
      out << "["<< intconstant_value(p.fixedarray.length) << "]";
      break;
    case OPTIONAL:
      emit_type(*p.optional);
      out << "*";
      break;
    case LENGTH:
      emit_array(*p.length.parser,name);
      break;

    case NUNION:
      {
        std::string first;
        int idx=0;
        FOREACH(elem, p.nunion){
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
      out << mk_str(p.ref.name)<<"* "; // TODO: Make ref more powerful, featuring ALIAS
      break;
    case NAME:
      out << mk_str(p.name.name);
      break;
    case APPLY:
      emit_type(*p.apply.inner); 
      break;
    }
  }
  void emit_type_definition( const parser &p,const std::string name)
  {
    switch(p.pr.N_type){
    case STRUCTURE:
    case ARRAY:
    case CHOICE:
      emit_type(p,name);
      out << ";" << std::endl;
      break;
    case WRAP:
      emit_type_definition(*p.pr.wrap.parser,name);
      break;
    default:
      break; //Other definitions don't need a type
    }
  }
  void emit_type( const definition &def){
    if(def.N_type != PARSER)
      return;
    std::string name(mk_str(def.parser.name));
    emit_type_definition(def.parser.definition,name);
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
        std::string name(mk_str(definition->parser.name));
        std::string post; 
        std::string typedef_t( typedef_type(definition->parser.definition,name,&post));
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
    if(val.ascii.N_type == ESCAPE)
      return boost::str(boost::format("'\\%c'") % val.ascii.escape);
    else
      return boost::str(boost::format("'%c'") % val.ascii.direct);
    break;
  case HEX:
    return std::string("0x") + mk_str(val.hex);
  case NUMBER:
    return mk_str(val.number);
  default:
    assert(!"Foo");
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
    case INTEGER:{
      int width = boost::lexical_cast<int>(mk_str(p.integer.parser.unsign));
      out << lval << "=" << int_expr(width) << ";\n";
      out<< "off += " << width<<";\n";
    }
      break;
    case STRUCTURE:
      FOREACH(field, p.structure){
        switch(field->N_type){
        case CONSTANT:
          action_constparser();
          break;
        case FIELD:{
          ValExpr fieldname(mk_str(field->field.name),&lval);
          action(field->field.parser->pr,fieldname);
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
      if(p.wrap.constbefore){
        FOREACH(c,*p.wrap.constbefore){
          action_constparser();
        }
      }
      action(p.wrap.parser->pr,lval);
      if(p.wrap.constafter){
        FOREACH(c,*p.wrap.constafter){
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
        FOREACH(c, p.choice){
          out << "case " << mk_str(c->tag) << ":\n";
          out << "tr = trace_begin + *tr;\n";
          out << ValExpr("N_type", &lval) << "= "<< mk_str(c->tag) <<";\n";
          ValExpr expr(mk_str(c->tag),&lval);
          action(c->parser->pr, expr );
          out << "break;\n";
        }
        out << "default: assert(\"BUG\");";
        out << "}";
      }      
      break;
    case NUNION:
      {
#ifdef DEBUG_OUT
        out << "fprintf(stderr,\"%d = choice %d %d\\n\",tr-trace_begin, tr[0], tr[1]);\n";
#endif
        int nr_option = 1;
        out << "switch(*(tr++)){\n";
        FOREACH(c, p.nunion){
          out << "case " << nr_option++ << ":\n";
          out << "tr = trace_begin + *tr;\n";
          action((*c)->pr, lval);
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
        action(p.apply.inner->pr, lval);
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
          switch(p.array.N_type){
          case MANYONE:
            i = p.array.manyone;break;
          case MANY:
            i = p.array.many; break;
          case SEPBY:
            i = p.array.sepby.inner; break;
          case SEPBYONE:
            i= p.array.sepbyone.inner; break;
          }
        }
        else{ /* LENGTH*/
          i = p.length.parser;
        }
          
        out << "{ /*ARRAY*/ \n pos save = 0;";
        out << count << "=" << "*(tr++);\n"
            << "save = *(tr++);\n";
        out <<data << "= " << "n_malloc("<<arena<<"," << count << "* sizeof(*"<<data<<"));\n"
            << "if(!"<< data<< "){return 0;}\n";

        std::string iter = boost::str(boost::format("i%d") % num_iters++);
        out << "for(pos "<<iter<<"=0;"<<iter<<"<"<<count<<";"<<iter<<"++){";
        if(p.N_type == ARRAY && (p.array.N_type == SEPBY || p.array.N_type == SEPBYONE)){
          out<< "if("<<iter<<">0){";
          action_constparser();
          out << "}";
        }
        ValExpr iexpr(iter);
        ArrayElemExpr elem(&data,&iexpr);
        action(i->pr,elem);
        out << "}\n";
        out << "tr = trace_begin + save;\n}";
      }
      break;
    case FIXEDARRAY:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      ValExpr iexpr(iter);
      ArrayElemExpr elem(&lval, &iexpr);
      out << "for(pos "<< iter<< "=0;"<<iter<<"<"<<intconstant_value(p.fixedarray.length) << ";"<<iter<<"++){";
      action(p.fixedarray.inner->pr,elem);
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
        action(p.optional->pr,deref);
        out << "}\n"
            << "else{tr = trace_begin + *tr;\n "<< lval <<"= NULL;}";
      }
      break;
    case REF:
      {
        out << lval << "= n_malloc("<<arena<<",sizeof(*"<<lval<<"));\n"
            << "if(!"<<lval<<"){return -1;}";
        out << "off = bind_"<<mk_str(p.ref.name)<< "("<<arena<<"," << lval << ", data,off,&tr,trace_begin);"
            << "if(parser_fail(off)){return -1;}\n";
        break;
      }
    case NAME:
      {
        out << "off = bind_"<<mk_str(p.name.name)<< "("<<arena<<",&" << lval << ", data,off,&tr,trace_begin);"
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
        std::string name= mk_str(def->parser.name);
        out << "static pos bind_"<< name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace,  pos * trace_begin);";
      }
    }
    FOREACH(def, grammar){
      if(def->N_type == PARSER){
        std::string name= mk_str(def->parser.name);
        out << "static pos bind_"<<name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace ,  pos * trace_begin){\n";
        out << " pos *tr = *trace;";
        //  out << name << "*ret = malloc(sizeof("<<name<<")); if(!ret) return -1;";
        ValExpr outexpr("out",NULL,1);
        action(def->parser.definition.pr,outexpr);
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
  //Negative of a constraint. 
  void constraint(std::string val, constraintelem &e){
    switch(e.N_type){
    case VALUE:
      out << val << "!="<< intconstant_value(e.value);
      break;
    case RANGE:
      out << "(";
      if(e.range.max){
        out << val << ">" << intconstant_value(*e.range.max);
      }
      else {
        out << "0";
      }
      out << "||";
      if(e.range.min){
        out << val << "<" << intconstant_value(*e.range.min);
      }
      else {
        out << "0";
      }
      out << ")";
      break;
    default:
      assert("!foo");
    }
  }
  void constraint(std::string val,  intconstraint &c){
    switch(c.N_type){
    case SINGLE:
      constraint(val,c.single);
      break;
    case SET:
      {
        int first = 0;
        FOREACH(allowed,c.set){
          if(first++ != 0)
            out << " && ";
          constraint(val,*allowed);
        }
      }
      break;
    case NEGATE:
      out << "!(";
      constraint(val,*c.negate);
      out << ")";
      break;
    }
  }
  
  void peg(const constrainedint &c, const std::string &fail){
    int width = boost::lexical_cast<int>(mk_str(c.parser.unsign));
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
      assert(mk_str(c.parser.unsign) == "8");
      FOREACH(ch, c.value.string){
        peg_constint(8,(boost::format("'%c'") % ch).str(),fail);
      }
      break;
    case VALUES:{
      int width = boost::lexical_cast<int>(mk_str(c.parser.unsign));
      FOREACH(v,c.value.values){
        peg_constint(width, intconstant_value(*v),fail);
      }
      break;
    }
    }
  }

  void peg_const(const constparser &c, const std::string &fail){
    switch(c.N_type){
    case CARRAY:
      peg_const(c.carray,fail);
      break;
    case CREPEAT:{
      std::string retrylabel = (boost::format("constmany_%d_repeat") % nr_const++).str();
      std::string faillabel = (boost::format("constmany_%d_end") % nr_const++).str();
      out << retrylabel << ":\n";
      peg_const(*c.crepeat,std::string("goto ")+faillabel+";");
      out << "goto " << retrylabel << ";\n";
      out << faillabel << ":\n";
      break;
    }
    case CINT:{
      int width = boost::lexical_cast<int>(mk_str(c.cint.parser.unsign));
      peg_constint(width,intconstant_value(c.cint.value),fail);
    }
      break;
    case CREF:
      out << "{\n"
          <<"pos ext = peg_" << mk_str(c.cref) << "(data,off,max);\n"
          <<"if(ext < 0){" << fail << "}\n"
          << "off = ext;\n"
          << "}\n";
      break;
    case CSTRUCT:
      FOREACH(field,c.cstruct){
        peg_const(*field,fail);
      }
      break;
    case CUNION:{
      out << "{\n"
          <<"backtrack back = stream_memo(str_current);";
      int thischoice = nr_choice++;
      std::string succ_label = boost::str(boost::format("cunion_%d_succ") % thischoice);
      int n_option = 1;
      FOREACH(option,c.cunion){
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
      peg_repeat(*array.manyone,fail, 1,NULL,scope);
      break;
    case MANY:
      peg_repeat(*array.many,fail, 0,NULL,scope);
      break;
    case SEPBYONE:
      peg_repeat(*array.sepbyone.inner,fail, 1,&array.sepbyone.separator,scope);
      break;
    case SEPBY:
      peg_repeat(*array.sepby.inner,fail, 0, &array.sepby.separator,scope);
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
    switch(parser.pr.N_type){
    case INTEGER:
      peg(parser.pr.integer,fail);
      break;
    case STRUCTURE:{
      Scope newscope(scope);
      FOREACH(field,parser.pr.structure){
        switch(field->N_type){
        case CONSTANT:
          peg(field->constant,fail);
          break;
        case FIELD:
          peg(*field->field.parser,fail,newscope);
          break;
        case DEPENDENCY:
          {
            //TODO: Use BIND/TYPE/ETC
            int this_dep = ++nr_dep;
            std::string depfail = (boost::format("goto faildep_%d;") % this_dep).str();
            std::string type_suffix;
            std::string type = typedef_type(*field->dependency.parser,"", &type_suffix);
            //            int width = boost::lexical_cast<int>(mk_str(field->dependency.parser.parser.unsign));
            std::string name(mk_str(field->dependency.name));
            out << "pos trace_" << name << " = n_trace_getpos(trace);\n";
            assert(type_suffix == "");

            ValExpr lval(std::string ("dep_") + name);
            peg(*field->dependency.parser, depfail, scope);
            dependency_action.action(field->dependency.parser->pr,lval); // TODO: Make action use a different arena
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
            FOREACH(stream, field->transform.left){
              body << "NailStream str_" << mk_str(*stream) <<";\n";
              // str_current cannot appear on the left
            }
            body << mk_str(field->transform.cfunction) << "_parse(arena";
            assert("!Check parameters");
            FOREACH(stream, field->transform.left){
              body << ", &str_"  << mk_str(*stream);
            }      
            header << "extern int " << mk_str(field->transform.cfunction) << "_parse(NailArena *tmp,";
            FOREACH(param, field->transform.right){
              switch(param->N_type){
              case PDEPENDENCY:{
                std::string name (mk_str(param->pdependency));
                //                if(newscope.dependency_type(name))
                //we need to emit a type for this function!
                body << "," << newscope.dependency_ptr(name); 
                header << "," << newscope.dependency_type(name) << "* " << name;
              }
                break;
              case PSTREAM:
                body << "," <<  newscope.stream_ptr(mk_str(param->pstream));  
                header << ",NailStream *" << mk_str(param->pstream);
                break;
              }
            }
            body << ");";
            header << (");");
            FOREACH(stream, field->transform.left){
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
      if(parser.pr.wrap.constbefore){
        FOREACH(c,*parser.pr.wrap.constbefore){
          peg(*c,fail);
        }
      }
      peg(*parser.pr.wrap.parser,fail,scope);
      if(parser.pr.wrap.constafter){
        FOREACH(c,*parser.pr.wrap.constafter){
          peg(*c,fail);
        }
      }
      break;
    case CHOICE:
      {
        parserlist l;
        FOREACH(c, parser.pr.choice){
          l.push_back(namedparser(mk_str(c->tag),c->parser));
        }
        peg_choice(l,fail,scope);
      }
      break;
    case NUNION:
      {
        parserlist l;
        int i = 0;
        FOREACH(c, parser.pr.nunion){
          i++;
          l.push_back(namedparser(boost::lexical_cast<std::string>(i),*c));
        }
        peg_choice(l,fail,scope);
      }
    case ARRAY:
      peg(parser.pr.array,fail,scope); 
      break;
    case FIXEDARRAY:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      
      out << "/*FIXEDARRAY*/ \n"
          << "for(pos "<<iter<<"=0;"<<iter<<"<"<<intconstant_value(parser.pr.fixedarray.length)<<";"<<iter<<"++){\n";
      peg(*parser.pr.fixedarray.inner,fail,scope);
      out << "}\n";
    }
      break;
    case LENGTH:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      out << "{/*LENGTH*/"
          << "pos many = n_tr_memo_many(trace);\n"
          << "pos count= "<< mk_str(parser.pr.length.length)<<";\n"
          << "pos "<<iter<<"=0;";
      out<< "for( "<<iter<<"=0;"<<iter<<"<count;"<<iter<<"++){";
      peg(*parser.pr.length.parser,fail,scope);
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
          << "str_current = "<< mk_str(parser.pr.apply.stream) << ";\n"
          << "n_tr_stream(n_trace, str_current)\n";
      peg(*parser.pr.apply.inner,(boost::format("goto fail_apply_%d") % this_many).str(), scope);
      out << "goto succ_apply_" << this_many << "\n";
      out << "fail_apply_" << this_many << ":\n";
      out << "str_current = orig_str; \n" 
          << fail << "\n";
      out << "succ_apply_" << this_many << ":\n"
          << "str_current = orig_str;\n";
      out << "}";        
    }
    case OPTIONAL:
      peg_optional(*parser.pr.optional,fail,scope);
      break;
      //TODO: Do caching here
    case NAME: // Fallthrough intentional, and kludgy
    case REF:
      //TODO: Deal with parameters here
      out << "i = peg_" << mk_str(parser.pr.ref.name)<< "(trace,data,off,max);";
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
      std::string name = mk_str(def.parser.name);
      //TODO: Parameters
      out <<"static pos peg_" << name <<"(n_trace *trace, NailStream *str_current){\n";
      out << "pos i;\n"; //Used in name and ref as temp variables
      peg(def.parser.definition, "goto fail;",scope);
      out << "return off;\n"
          << "fail:\n return -1;\n";
      out << "}\n";
    }
    else if(def.N_type == CONSTANTDEF){
      std::string name = mk_str(def.constantdef.name);
      out << "static pos peg_" << name << "(NailStream *str_current){\n";
      peg_const(def.constantdef.definition, "goto fail;");
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
        out << "static pos peg_" << mk_str(def->parser.name) <<"(n_trace *trace,NailStream *str_current);\n";
        break;
      case CONSTANTDEF:
        out << "static pos peg_" << mk_str(def->constantdef.name) <<"(NailStream *str_current);\n";
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
      std::string name= mk_str(def->parser.name);
      *out << name << "*parse_" << name << "(NailArena *arena, const uint8_t *data, size_t size);\n";
    }
  }

}


