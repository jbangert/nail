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
void emit_hammer_parser(std::ostream *out, grammar *grammar, const char *header){
  assert(out->good());

  //Emit forward declarations
  FOREACH(definition,*grammar){
    if(definition->N_type!=PARSER)
      continue;
    std::string name(mk_str(definition->PARSER.name));
    *out << "typedef "<< typedef_type(definition->PARSER.definition,name)<< " " << name << ";" << std::endl;
  }
  FOREACH(definition, *grammar){
    switch(definition->N_type){
    case PARSER:
      
      break;
    case CONST:
      break;
    }
  }
 
}
