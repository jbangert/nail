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

std::string intconstant_value(const intconstant &val){
  switch(val.N_type){
  case ASCII:
    if(val.ASCII.N_type == escape)
      return boost::str(boost::format("'\\%c'") % val.ASCII.escape);
    else
      return boost::str(boost::format("'%c'") % val.ASCII.direct);
    break;
  case NUMBER:
    return mk_str(val.NUMBER);
  }
}
class CPrimitiveParser{
  std::ostream &out;
  int nr_choice,nr_option;
  int nr_many;
  int nr_const;
  
  int bit_offset = 0; // < 0 = unknown alignment, 0 == byte aligned, 1 = byte + 1, etc 
  CPrimitiveParser(std::ostream *os) : out(*os), nr_choice(1),nr_many(0),nr_const(0){}
  std::string choice_label(){return  (boost::format("choice_%u_%u") % nr_choice %n_option++).str();}
  /*Packrat pass emits a light-weight trace of data structure*/
  //TODO: Keep track of bit alignment
  void check_int(unsigned int width, const std::string &fail){
    out << "if(off + "<< width <<">= max) {fail}";
  }
  std::string int_expr(unsigned int width){
    if(width>=64){
      throw std::runtime_error("More than 64 bits in integer");
    }
    //TODO: x
    // TODO: Make big endian. Flip n bytes
    return boost::str(boost::format("BITSLICE(*(uint64_t *)(data+(off % ~7)),off & 7 + %d, off&7)") % bytes);
  }
  void constraint( const intconstraint &constraint){
      switch(constraint.N_type){
      case RANGE:
        out << "val>"<<intconstant_value(constraint.RANGE.max) << "|| val < "<<intconstant_value(constraint.RANGE.min);
        break;
      case SET:
        {
          int first = 0;
          FOREACH(val,constraint.SET){
            if(first++ != 0)
              out << " && ";
            out << "val != "<<intconstant_value(val);
          }
        }
        break;
      case NEGATE:
        out << "!(";
        constraint(!constraint.NEGATE);
        out << ")";
        break;
      }
  }
  
  void packrat(const constrainedint &c, const std::string &fail){
    int width = boost::lexical_cast<int>(mk_str(c.parser.UNSIGNED));
    check_int(width,fail);
    if(c.constraint != NULL){
      out << "{\n uint64 val = "<< int_expr(width) << ";\n";
      out << "if(";
      constraint(*c.constraint);
      out << "){"<<fail<<"}\n";
      out << "}\n";
    }
    out << "pos +="<< width<<";\n";
  }
  void packrat_constint(int width, std::string value,std::string &fail){
      check_int(width,fail);
      out << "if( "<< int_expr(width) << "!= "<<value"){"<<fail<<"}";
      out << "off += " << width; //TODO deal with bits
      out << "end_of_const = pos;"
  }
  void packrat_const(const constarray &c, const std::string &fail){
    switch(c.value.N_type){
    case STRING:
      assert(mk_str(c.parser.UNSIGNED) == "8");
      FOREACH(ch, c.value.STRING){
        packrat_constint(8,std::string(ch),fail);
      }
      break;
    case VALUES:{
      int width = boost::lexical_cast<int>(mk_str(c.parser.UNSIGNED));
      FOREACH(v,c.value.VALUES){
        packrat_constint(width, intconstant_value(v),fail);
      }
      break;
    }
      
    }

  }

  void packrat_const(const constparser &c, const std::string &fail){

    switch(c.N_type){
    case CARRAY:
      packrat_const(c.CARRAY);
      break;
    case CREPEAT:{
      std::string retrylabel = (boost::format("constmany_%d_repeat") % nr_const++).str();
      std::string faillabel = (boost::format("constmany_%d_end") % nr_const++).str();
      out << retrylabel << ":\n";
      packrat_const(c.CREPEAT,std::string("goto ")+faillabel+";");
      out << "goto " << retrylabel << ";\n";
      out << faillabel << ":\n";
      break;
    }
    case CINT:{
      int width = boost::lexical_cast<int>(mk_str(c.CINT.parser.UNSIGNED));
      long long value = boost::lexical_cast<long long>();
      packrat_constint(width,mk_str(c.CINT.value),fail);
    }
    break;
    case CREF:
      out << "{\n"
          <<"pos ext = packrat_const_" << make_str(c.CREF) << "();"
          <<"if(ext < 0){" << fail << "}\n"
          << "end_of_const = ext;"
          << "}";
      break;
    case CSTRUCT:
      FOREACH(field,c.CSTRUCT){
        packrat_const(field,fail);
      }
        break;
    }

  }
  void packrat(const constparser &c, const std::string &fail){
    out << "{ pos end_of_const = off;";
    packrat_const(c,fail);
    out << "if(parser_fail(n_tr_const(trace,end_of_const))){"<<fail<<"}\n"
    out << "}";    
  }
  typedef  std::list<const parser &> parserlist;
  void packrat_choice(const parserlist &list, const std::string &fail){
    nr_choice++;
    nr_option = 0;
    out << "{pos backtrack = off;\n"
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
      out << "trace->iter = backtrack;\n";
    }
    out << fail << "\n";
    out << success_label <<":\n";
  }
  void packrat_repeat(const parser &p, const std::string &fail, size_t min, const constparser *separator){
    std::string gotofail=(boost::format("goto fail_repeat_%d;") % nr_many).str();
    out << "{\n";
    out << "pos many = n_tr_memo_many(n_trace *trace);\n"
        << "pos count = 0;\n"
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
      //TODO: Do caching here
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
