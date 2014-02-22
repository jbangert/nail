#include "nailtool.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}

#define mk_str(x) std::string((const char *)x.elem,x.count)
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
void emit_type(std::ostream &out, const parser &outer,const std::string name =""){
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
    out << "struct" << name<< "{\n enum {";
    int idx=0;
    FOREACH(option, p.CHOICE){
      if(idx++ >0) 
        out << ',';
      out << mk_str(option->tag);
    }
    out << "} N_tag; \n N_tag N_type; \n";
    out << "union {";
    FOREACH(option, p.CHOICE){
      emit_type(out,*option->parser);
      out << " "<<  mk_str(option->tag) << ";\n";
    }
    out<< "}\n}\n";
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
    out << "}" << std::endl;
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
void emit_type_definition(std::ostream &out, const parser &p,const std::string name)
{

  switch(p.PR.N_type){
  case STRUCT:
  case ARRAY:
  case CHOICE:
    emit_type(out,p,name);
    break;
  case WRAP:
    emit_type(out,*p.PR.WRAP.parser,name);
    break;
  default:
    break; //Other definitions don't need a type
  }
}
void emit_type(std::ostream &out, const definition &def){
  if(def.N_type != PARSER)
    return;
  std::string name(mk_str(def.PARSER.name));
  emit_type_definition(out,def.PARSER.definition,name);
}

void emit_parser(std::ostream *out, grammar *grammar, const char *header){
  try{
  assert(out->good());

  //Emit forward declarations
  FOREACH(definition,*grammar){
    if(definition->N_type!=PARSER)
      continue;
    std::string name(mk_str(definition->PARSER.name));
    *out << "typedef "<< typedef_type(definition->PARSER.definition,name)<< " " << name << ";" << std::endl;
  }
  FOREACH(definition, *grammar){
    emit_type(*out,*definition);
  }
  *out<< std::endl;
  } catch(std::exception &e){
    std::cerr << "Exception while generating parser" << e.what()<<std::endl;
    exit(-1);
  }
}
