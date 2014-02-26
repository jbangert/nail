#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}


#define mk_str(x) std::string((const char *)x.elem,x.count)
static const std::string parser_template(_binary_parser_template_c_start,_binary_parser_template_c_end - _binary_parser_template_c_start);
class CDataModel{
  std::ostream &out;
  std::string int_type(const intp &intp ){
    int length = boost::lexical_cast<int>(mk_str(intp.SIGNED));
    if(length<=8) length = 8;
    else if(length<=16) length=16;
    else if(length<=32) length=32;
    else if(length<=64) length=64;
    else throw std::range_error("Integer longer than 64 bits");
    return (boost::format("%s%d_t") % (intp.N_type == UNSIGNED ? "uint":"int" )% length).str();
  }

  // Gets the type for a top-level parser (to be used in the typedef)
  std::string typedef_type(const parser &p, std::string name){
    const parserinner &inner(p.N_type == PR ? p.PR : p.PAREN);
    switch(inner.N_type){
    case INT:
      return int_type(inner.INT.parser);
    case WRAP:
      return typedef_type(*inner.WRAP.parser,name);
    case OPTIONAL:
      return (boost::format("%s*") % typedef_type(*inner.OPTIONAL,name)).str();
    case NAME:
      return mk_str(inner.NAME);
    case REF:
      return (boost::format("%s *") % mk_str(inner.NAME)).str();
    case UNION:
      /*   FOREACH(parser,inner.UNION){
           if(!type_equal(inner.UNION.elem[0],parser))
           throw std::runtime_error("Grammar invalid at UNION");
           } */
      return typedef_type(*inner.UNION.elem[0],name);
    case STRUCT:
    case ARRAY:
    case CHOICE:
      return (boost::format("struct %s") % name).str();
 
    }
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
        emit_type(out, *field->FIELD.parser);
        out << " " << mk_str(field->FIELD.name) << ";\n";
      }
      out << "}" << std::endl;
      break;
    case WRAP:
      emit_type(out,*p.WRAP.parser);
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
        emit_type(out,*option->parser);
        out << " "<<  mk_str(option->tag) << ";\n";
      }
      out<< "};\n}";
    }
      break;
  
    case ARRAY:
      out << "struct "<< name <<"{\n";
      switch(p.ARRAY.N_type){
      case MANY:
      case MANYONE:
        emit_type(out,*p.ARRAY.MANY);
        break;
      case SEPBY:
      case SEPBYONE:
        emit_type(out,*p.ARRAY.SEPBY.inner);
        break;
      }
      out << "*elem;\n size_t count;\n";
      out << "}" ;
      break;
    case OPTIONAL:
      emit_type(out,*p.OPTIONAL);
      out << "*";
      break;
    case UNION:
      {
        std::string first;
        int idx=0;
        FOREACH(elem, p.UNION){
          std::stringstream tmp;
          emit_type(tmp,**elem);
          if(idx++==0)
            first = tmp.str();
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
      out << mk_str(p.REF)<<"* ";
      break;
    case NAME:
      out << mk_str(p.NAME);
      break;
    }
  }
  void emit_type_definition( const parser &p,const std::string name)
  {
    switch(p.PR.N_type){
    case STRUCT:
    case ARRAY:
    case CHOICE:
      emit_type(out,p,name);
      out << ";" << std::endl;
      break;
    case WRAP:
      emit_type_definition(out,*p.PR.WRAP.parser,name);
      break;
    default:
      break; //Other definitions don't need a type
    }
  }
  void emit_type( const definition &def){
    if(def.N_type != PARSER)
      return;
    std::string name(mk_str(def.PARSER.name));
    emit_type_definition(out,def.PARSER.definition,name);
  }
public:
  void emit_parser( grammar *grammar){
    try{
      assert(out.good());
      out << "#include <stdint.h>\n #include <string.h>\n";
      //Emit forward declarations
      FOREACH(definition,*grammar){
        if(definition->N_type!=PARSER)
          continue;
        std::string name(mk_str(definition->PARSER.name));
        out << "typedef "<< typedef_type(definition->PARSER.definition,name)<< " " << name << ";" << std::endl;
      }
      FOREACH(definition, *grammar){
        emit_type(out,*definition);
      }
      out<< std::endl;
    } catch(std::exception &e){
      std::cerr << "Exception while generating parser" << e.what()<<std::endl;
      exit(-1);
    }
  }

  CDataModel(std::ostream *os) : out(*os){}
  
};
class CPrimitiveParser{
  std::ostream &out;
  int nr_choice,nr_option;
  int nr_many;
  CPrimitiveParser(std::ostream *os) : out(*os), nr_choice(1),nr_many(0){}
  std::string choice_label(){return  (boost::format("choice_%u_%u") % nr_choice %n_option++).str();}
  /*Packrat pass emits a light-weight trace of data structure*/
  void packrat(const constrainedint &c, const std::string &fail){
    
  }
  void packrat(const constparser &c, const std::string &fail){
  }
  typedef  std::list<const parser &> parserlist;
  void packrat_choice(const parserlist &list, const std::string &fail){
    nr_choice++;
    nr_option = 0;
    out << "{pos backtrack = trace->iter;\n"
        << "pos choice_begin = n_tr_begin_choice(trace); \n"
        << "pos choice;\n"
        << "if(parser_fail(choice_begin)) {" << fail <<"}\n";
    int i=0;
    std::string success_label = (boost::format("choice_%d_succ") % nr_choice).str();
    BOOST_FOREACH(parser &p, list){
      std::string fallthrough_memo  = choice_label();
      std::string fallthrough_goto = boost::str(boost_format("goto %s") % fallthrough_memo);
      out << "choice = n_tr_memo_choice(trace);\n";
      packrat(p,fallthrough_goto);
      out << "goto " << success_label<< "\n";
      out << fallthrough_memo << ":\n";
    }
    out << fail << "\n";
    out << success_label <<":\n";
  }
  void packrat_repeat(const parser &p, const std::string &fail, size_t min, const constparser *separator){
    std::string gotofail=(boost::format("goto fail_repeat_%d;") % nr_many).str();
    out << "{\n";
    out << "pos many = n_tr_memo_many(n_trace *trace);\n"
        << "pos count = 0;"
        << "succ_optional_" << nr_many << ":";
    if(separator != NULL){
      out << "if(count>0){\n";
      packrat(*separator,gotofail);
      out << "}\n";
    }
    packrat(p, gotofail);
    out << "count++;\n";
    out << " goto succ_optional_"<< nr_many << ";\n";
    out << "fail_optional_" << nr_many << ":\n";
    if(min>0){
      out << "if(count < min){"<< fail << "}\n";
    }
    out << "n_tr_write_many(trace,many,count);\n"
    nr_many++;          
  }
  void packrat_array(const arrayparser &array, const std::string &fail){
    switch(array.N_type){
    case MANYONE:
      packrat_repeat(*array.MANYONE,fail, 1,NULL);
      break;
    case MANY:
      packrat_repeat(*array.MANY,fail, 0,NULL);
      break;
    case SEPBYONE:
      packrat_repeat(*array.SEPBYONE.inner,fail, 1,&array.SEPBYONE.separator);
      break;
    case SEPBY:
      packrat_repeat(*array.SEPBY,fail, 0, &array.SEPBY.separator);
      break;
    }
    arrayparser
  }
  void packrat_optional(const parser &p, const std::string &fail){
    out << "{\n";
    out << "pos many = n_tr_memo_many(n_trace *trace);\n";
    packrat(p, (boost::format("goto fail_optional_%d;") % nr_many).str());
    out << "n_tr_write_many(trace,many, 1); goto succ_optional_"<< nr_many << ";\n";
    out << "fail_optional_" << nr_many << ":\n";
    out << "n_tr_write_many(trace,many,0);\n"
        << "succ_optional_" << nr_many << ":\n";
    nr_many++;      
  }
  void packrat(const parser &parser, const std::string &fail ){
    switch(parser.PR.N_type){
    case INT:
      packrat(parser.PR.INT,fail);
      break;
    case STRUCT:
      FOREACH(field,parser.PR.STRUCT){
        switch(field.N_type){
        case CONSTANT:
          packrat(field.CONSTANT,fail);
          break;
        case FIELD:
          packrat(field.PARSER.parser,fail);
          break;
        }
      }
      break;
    case WRAP:
      if(parser.PR.WRAP.constbefore){
        FOREACH(c,parser.PR.WRAP.constbefore){
          packrat(c,fail);
        }
      }
      packrat(parser.PR.WRAP.parser);
      if(parser.PR.WRAP.constafter){
        FOREACH(c,parser.PR.WRAP.constafter){
          packrat(c,fail);
        }
      }
      break;
    case CHOICE:
      {
        parserlist l;
        FOREACH(c, parser.PR.CHOICE){
          l.push_back(c.parser);
        }
        packrat_choice(l,fail);
      }
      break;
    case UNION:
      {
        parserlist l;
        FOREACH(c, parser.UNION){
          l.push_back(c);
        }
        packrat_choice(l,fail);
      }
    case ARRAY:
      packrat(parser.PR.ARRAY,fail);
      break;
    case OPTIONAL:
      packrat(parser.PR.OPTIONAL,fail);
      break;
    case NAME: // Fallthrough intentional, and kludgy
    case REF: 
      out << "i = packrat_" << parser.PR.REF.name<< "(tmp, trace,data,pos,max);";
      out << "if(parser_fail(i)){" << fail << "}\n";
          << "else{off=i;}";
      break;
    }
  }
  void packrat(const definition &def) {
    std::string name = mk_str(def.name);
    out <<"static int packrat_" << name <<"(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max){\n";
    out << "pos i;\n"; //Used in name and ref as temp variables
    out << "}\n";
  }
};
void emit_parser(std::ostream *out, grammar *grammar){
  CDataModel data(out);
  CPrimitiveParser p(out);
  data.emit_parser(grammar);
}
