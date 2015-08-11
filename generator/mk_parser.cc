#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <list>
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}
#undef DEBUG_OUT
//static const std::string parser_template(_binary_parser_template_c_start,_binary_parser_template_c_end - _binary_parser_template_c_start);

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

static std::string int_expr(const char * stream,unsigned int width,Endian endian){
  if(width>64){
    throw std::runtime_error("More than 64 bits in integer");
  }
  //TODO: Support little endian?
  if(endian == Big){
    return boost::str(boost::format("read_unsigned_bits(%s,%d)")%stream % width);
  } else { 
    return boost::str(boost::format("read_unsigned_bits_littleendian(%s,%d)")%stream % width);    
  }
}

class CAction{
  std::ostream &out;
  int num_iters;
  std::string arena; // Which arena to allocate from (local variable name)
  void action_constparser(){
#ifdef DEBUG_OUT
    out << "fprintf(stderr,\"%d = const %d\\n\",tr-trace_begin, *tr);\n";
#endif
    out << "stream_reposition(stream,*tr);\n"
        << "tr++;\n";
  }
public:
  bool big_endian;
  void action (const parserinner &p, Expr &lval, Endian end){
    switch(p.N_type){
    case INTEGER:{
      int width = boost::lexical_cast<int>(mk_str(p.integer.parser.unsign));
      out << lval << "=" << int_expr("stream",width,end) << ";\n";
    }
      break;
    case STRUCTURE:
      FOREACH(field, p.structure){
        switch(field->N_type){
        case CONSTANT:
          action_constparser();
          break;
        case FIELD:{
          std::string name = mk_str(field->field.name);
          ValExpr fieldname(name,&lval);
          action(field->field.parser->pr,fieldname,end);
          break;
        }
        case DEPENDENCY:{
          //TODO: Be more efficient
          action_constparser();

        }
          break;
        case TRANSFORM:
          out << "*stream = *(NailStream *)tr;";
#ifdef DEBUG_OUT
          out << "fprintf(stderr,\"%d = stream %ul\\n\",tr-trace_begin, stream->pos);\n";
#endif
          out << "tr+= sizeof(NailStream) / sizeof(*tr);";
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
      action(p.wrap.parser->pr,lval,end);
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
          std::string tag (mk_str(c->tag));
          boost::algorithm::to_lower(tag);
          ValExpr expr(tag,&lval);
          action(c->parser->pr, expr,end );
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
          action((*c)->pr, lval,end);
          out << "break;\n";
        }
        out << "default: assert(!\"Error\"); exit(-1);";
        out << "}";
      }
      break;
    case APPLY:
      {
        out << "{/*Apply*/ "
            <<"NailStream *original_stream = stream;\n;";
        out << "stream = (NailStream *)tr;";
#ifdef DEBUG_OUT
        out << "fprintf(stderr,\"%d = stream %ul\\n\",tr-trace_begin, stream->pos);\n";
#endif
        out << "tr+= sizeof(NailStream) / sizeof(*tr);";
        action(p.apply.inner->pr, lval,end);
        out << "stream = original_stream;";
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
        out <<data << "= " << "(typeof("<<data<<"))n_malloc("<<arena<<"," << count << "* sizeof(*"<<data<<"));\n";

        std::string iter = boost::str(boost::format("i%d") % num_iters++);
        out << "for(pos "<<iter<<"=0;"<<iter<<"<"<<count<<";"<<iter<<"++){";
        if(p.N_type == ARRAY && (p.array.N_type == SEPBY || p.array.N_type == SEPBYONE)){
          out<< "if("<<iter<<">0){";
          action_constparser();
          out << "}";
        }
        ValExpr iexpr(iter);
        ArrayElemExpr elem(&data,&iexpr);
        action(i->pr,elem,end);
        out << "}\n";
        out << "tr = trace_begin + save;\n}";
      }
      break;
    case FIXEDARRAY:{
      std::string iter = boost::str(boost::format("i%d") % num_iters++);
      ValExpr iexpr(iter);
      ArrayElemExpr elem(&lval, &iexpr);
      out << "for(pos "<< iter<< "=0;"<<iter<<"<"<<intconstant_value(p.fixedarray.length) << ";"<<iter<<"++){";
      action(p.fixedarray.inner->pr,elem,end);
      out << "}";
    }
      break;
    case OPTIONAL:
      {
        out<< "if(*tr<0) /*OPTIONAL*/ {\n"
           << "tr++;\n"
           << lval << "= " << "(typeof("<<lval<<"))n_malloc("<<arena<<",sizeof(*"<<lval<<"));\n";
        DerefExpr deref(lval);
        action(p.optional->pr,deref,end);
        out << "}\n"
            << "else{tr = trace_begin + *tr;\n "<< lval <<"= NULL;}";
      }
      break;
    case REF:
      {
        out << lval << "= (typeof("<<lval<<")) n_malloc("<<arena<<",sizeof(*"<<lval<<"));\n";
        out << "if(parser_fail(bind_"<<mk_str(p.ref.name)<< "("<<arena<<"," << lval << ", stream,&tr,trace_begin)))"
            << "{return -1;}\n";
        break;
      }
    case NAME:
      {
        out << "if(parser_fail(bind_"<<mk_str(p.name.name)<< "("<<arena<<",&" << lval << ", stream,&tr,trace_begin))) { return -1;}";

        break;
      }
    }
  }
public:
  //Refactor this. make emit_action static, allocation its own CAction to build stuff
  CAction(std::ostream *os,std::string _arena): out(*os), num_iters(1),arena(_arena){}
 
  void emit_action(const grammar &grammar){
    Endian end;
    FOREACH(def, grammar){
      if(def->N_type == PARSER){ 
        std::string name= mk_str(def->parser.name);
        out << "static pos bind_"<< name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace,  pos * trace_begin);";
      }
    }
    FOREACH(def, grammar){
      if(def->N_type == ENDIAN){
        end = def->endian.N_type == BIG ? Big : Little;
      }
        
      if(def->N_type == PARSER){
        std::string name= mk_str(def->parser.name);
        out << "static int bind_"<<name<< "(NailArena *arena," << name <<"*out,NailStream *stream, pos **trace ,  pos * trace_begin){\n";
        out << " pos *tr = *trace;";
        //  out << name << "*ret = malloc(sizeof("<<name<<")); if(!ret) return -1;";
        ValExpr outexpr("out",NULL,1);
        action(def->parser.definition.pr,outexpr,end);
        out << "*trace = tr;";
        out<< "return 0;}";
        if(!def->parser.parameters || def->parser.parameters->count==0){
          out << name << "*parse_" << name << "(NailArena *arena, const uint8_t *data, size_t size){\n"
              << "NailStream stream = {.data = data, .pos= 0, .size = size, .bit_offset = 0};\n"
              << "NailArena tmp_arena;"
              << "NailArena_init(&tmp_arena, 4096, arena->error_ret);"
              <<"n_trace trace;\n"
              <<"pos *tr_ptr;\n pos pos;\n"
              << name <<"* retval;\n"
              << "n_trace_init(&trace,4096,4096);\n"
              << "if(parser_fail(peg_"<<name<<"(&tmp_arena,&trace,&stream))) goto fail;"
              << "if(stream.pos != stream.size) goto fail; "
              << "retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));\n"
              << "stream.pos = 0;\n"
              << "tr_ptr = trace.trace;"
              << "if(bind_"<<name<<"(arena,retval,&stream,&tr_ptr,trace.trace) < 0) goto fail;\n"
              << "out: n_trace_release(&trace);\n"
              << "NailArena_release(&tmp_arena);"
              <<"return retval;"
              << "fail: retval = NULL; goto out;"
              <<"}";
        }
      }
      out << std::endl;
    }
  }
};

class CPrimitiveParser{
  std::stringstream out;
  std::ostream &final,&hdr;
  std::stringstream header;
  std::stringstream forward_declarations;
  int nr_choice,nr_option;
  int nr_many;
  int nr_const;
  int nr_dep;
  int num_iters;
  int bit_offset = 0; // < 0 = unknown alignment, 0 == byte aligned, 1 = byte + 1, etc 
  Endian end;
  CAction dependency_action;
  /*Parse pass emits a light-weight trace of data structure*/
  //TODO: Keep track of bit alignment
  void check_int(unsigned int width, const std::string &fail){
    out << "if(parser_fail(stream_check(str_current,"<<width<<"))) {"<<fail<<"}\n";
  }
  
  
  void peg(const constrainedint &c, const std::string &fail){
    int width = boost::lexical_cast<int>(mk_str(c.parser.unsign));
    check_int(width,fail);
    if(c.constraint != NULL){
      out << "{\n uint64_t val = "<< int_expr("str_current",width,end) << ";\n";
      out << "if(";
      constraint(out,std::string("val"),*c.constraint);
      out << "){"
          << "stream_backup(str_current,"<<width<<");"
          <<fail<<"}\n";
      out << "}\n";
    }
    else{
      out << "stream_advance(str_current,"<<width<<");\n";
    }
  }
  void peg_constint(int width, const std::string value,const std::string &fail){
    check_int(width,fail);
    out << "if( "<< int_expr("str_current",width,end) << "!= "<<value<<"){"
          << "stream_backup(str_current,"<<width<<");"
        <<fail<<"}";
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
      out <<"if(parser_fail(peg_" << mk_str(c.cref) << "(str_current))){\n"
          <<fail <<  "}\n";

      break;
    case CSTRUCT:
      FOREACH(field,c.cstruct){
        peg_const(*field,fail);
      }
      break;
    case CUNION:{
      out << "{\n"
          <<"NailStreamPos back = stream_getpos(str_current);";
      int thischoice = nr_choice++;
      std::string succ_label = boost::str(boost::format("cunion_%d_succ") % thischoice);
      int n_option = 1;
      FOREACH(option,c.cunion){
        std::string fallthrough_label = boost::str(boost::format("cunion_%d_%d") % thischoice % n_option++);
        std::string failopt = boost::str(boost::format("goto %s;") % fallthrough_label);
        peg_const(*option,failopt);
        out << "goto " << succ_label << ";\n";
        out << fallthrough_label << ":\n";
        out << "stream_reposition(str_current,back);\n";
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
    out << "if(parser_fail(n_tr_const(trace,str_current))){"<<fail<<"}\n";
  }
  struct namedparser{
    const parser *p;
    std::string name;
    namedparser(std::string _name, const parser *_p): p(_p), name(_name){
    }
  };
  typedef  std::list<namedparser> parserlist;
  void peg_choice(const parserlist &list, const std::string &fail, const Scope &scope){
    int this_choice = nr_choice++;
  
    out << "{NailStreamPos back = stream_getpos(str_current);\n"
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
      out << "stream_reposition(str_current, back);\n";

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
            out << "; \n pos trace_" << name << " = n_trace_getpos(trace);\n";
            assert(type_suffix == "");

            ValExpr lval(std::string ("dep_") + name);
            out << type << " dep_" << name << ";\n";
            out  << "{\n"
                 << "NailStream tmpstream = *str_current;\n";
            peg(*field->dependency.parser, fail, scope);
            out  << "NailStream *stream = &tmpstream;"; //TODO: parametrize this on action
            dependency_action.action(field->dependency.parser->pr,lval,end); 
            out << "}";
            newscope.add_dependency_definition(name,type, 0);
            out << "n_tr_setpos(trace,trace_"<<name<<");\n";
            out << "n_tr_const(trace,str_current);\n";
          }
          break;
          
        case TRANSFORM:
          {
            header << "extern int " << mk_str(field->transform.cfunction) << "_parse(NailArena *tmp";
            FOREACH(stream, field->transform.left){
              out << ";\nNailStream str_" << mk_str(*stream) <<";\n"; // If declaration occurs right
                                                                      // after a label, we need an
                                                                      // empty statement
              // str_current cannot appear on the left
            }
            out << "if(";
            out << mk_str(field->transform.cfunction) << "_parse(tmp_arena"; //TODO: use temp arena

            FOREACH(stream, field->transform.left){           
              header << ",NailStream *str_" << mk_str(*stream);
              out << ", &str_"  << mk_str(*stream);
            }      
            FOREACH(param, field->transform.right){
              switch(param->N_type){
              case PDEPENDENCY:{
                std::string name (mk_str(param->pdependency));
                //                if(newscope.dependency_type(name))
                //we need to emit a type for this function!
                out << "," << newscope.dependency_ptr(name); 
                header << "," << newscope.dependency_type(name) << "* " << name;
              }
                break;
              case PSTREAM:
                out << "," <<  newscope.stream_ptr(mk_str(param->pstream));  
                header << ",NailStream *" << mk_str(param->pstream);
                break;
              }
            }
            out << ")) {" << fail << "}";
            header << ");\n";
            FOREACH(stream, field->transform.left){
              newscope.add_stream_definition(mk_str(*stream));
            }
            out << "n_tr_stream(trace,str_current);";
            
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
          << "pos count= dep_"<< mk_str(parser.pr.length.length)<<";\n"
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
      //TODO: How to backtrack within this stream?
      out << "{/*APPLY*/";
      out << "NailStream  * orig_str = str_current;\n"
          << "str_current = "<< scope.stream_ptr(mk_str(parser.pr.apply.stream)) << ";\n"
          << "n_tr_stream(trace, str_current);\n";
      peg(*parser.pr.apply.inner,(boost::format("goto fail_apply_%d;") % this_many).str(), scope);
      out << "goto succ_apply_" << this_many << ";\n";
      out << "fail_apply_" << this_many << ":\n";
      out << "str_current = orig_str; \n" 
          << fail << "\n";
      out << "succ_apply_" << this_many << ":\n"
          << "str_current = orig_str;\n";
      out << "}";        
    }
      break;
    case OPTIONAL:
      peg_optional(*parser.pr.optional,fail,scope);
      break;
      //TODO: Do caching here
    case NAME: // Fallthrough intentional, and kludgy
    case REF:
      {
        std::string parameters =   parameter_invocation(parser.pr.ref.parameters,scope);
        out << "if(parser_fail(peg_" << mk_str(parser.pr.ref.name)<< "(tmp_arena,trace,str_current"<<parameters<<"))) {";
        out << fail << "}\n";
      }
      break;
    }
  }
public:
  CPrimitiveParser(std::ostream *os, std::ostream *header) : final(*os), hdr(*header),nr_choice(1),nr_many(0),nr_const(0),nr_dep(0),num_iters(1), dependency_action(&out,"tmp_arena"), end(Big){}
  void peg(const definition &def) {
    if(def.N_type == ENDIAN){  
      end = def.endian.N_type == BIG ? Big : Little;
    } else if(def.N_type == PARSER){
      Scope scope;
      std::string name = mk_str(def.parser.name);
      scope.add_stream_parameter("current");
      std::string params = parameter_definition(def,scope);
      out <<"static pos peg_" << name <<"(NailArena *tmp_arena,n_trace *trace, NailStream *str_current"<<
        params <<"){\n";
      out << "pos i;\n"; //Used in name and ref as temp variables
      peg(def.parser.definition, "goto fail;",scope);
      out << "return 0;\n"
          << "fail:\n return -1;\n";
      out << "}\n";
      forward_declarations << "static pos peg_" << mk_str(def.parser.name) <<"(NailArena *tmp_arena,n_trace *trace,NailStream *str_current"<<params<<");\n";

    }
    else if(def.N_type == CONSTANTDEF){
      std::string name = mk_str(def.constantdef.name);
      out << "static pos peg_" << name << "(NailStream *str_current){\n";
      peg_const(def.constantdef.definition, "goto fail;");
      out << "return 0;\n"
          << "fail: return -1;\n"
          << "}";
      forward_declarations << "static pos peg_" << mk_str(def.constantdef.name) <<"(NailStream *str_current);\n";
    }
  }
  void emit_parser(const grammar &gram){
    FOREACH(def, gram){
      peg(*def);
    }
    out << std::endl;
    final << header.str() << forward_declarations.str() << out.str() << std::endl;
    hdr << header.str() << std::endl;

  }
};
void emit_parser(std::ostream *out, std::ostream *header,grammar *grammar){

  CPrimitiveParser p(out,header);
  CAction a(out,"arena");
  *out << std::string((char *)parser_template_c,parser_template_c_len);
  *out << std::string((char *)parser_template_cc, parser_template_cc_len);
  
  p.emit_parser(*grammar);
  a.emit_action(*grammar);
}


