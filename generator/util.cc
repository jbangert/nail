#include "nailtool.h"
bool parameter_type_check(parameterlist *param, parameterdefinitionlist *def, const Scope &scope){
  if(param == NULL && def  == NULL)
    return true;
  if(param != NULL || def != NULL)
    return false;
  if(param->count != def->count)
    return false;
  for(int i =0; i<param->count; i++)
  {
    if(param->elem[i].N_type != def->elem[i].N_type)
      return false;
    switch(param->elem[i].N_type){
    case DSTREAM:
      if(mk_str(param->elem[i].pstream) != mk_str(def->elem[i].dstream))
        return false;
      break;
    case DDEPENDENCY:{
      std::string post;
      if(scope.dependency_type(mk_str(param->elem[i].pdependency)) != typedef_type(*def->elem[i].ddependency.type,"",&post) || post != "")
        return false;
      break;
    }
    }
  }
  return true;
}  
std::string int_type(const intp &intp ){
    int length = boost::lexical_cast<int>(mk_str(intp.sign));
    if(length<=8) length = 8;
    else if(length<=16) length=16;
    else if(length<=32) length=32;
    else if(length<=64) length=64;
    else throw std::range_error("Integer longer than 64 bits");
    return (boost::format("%s%d_t") % (intp.N_type == UNSIGN ? "uint":"int" )% length).str();
  }

  // Gets the type for a top-level parser (to be used in the typedef)
std::string typedef_type(const parser &p, std::string name, std::string *post){
    const parserinner &inner(p.N_type == PR ? p.pr : p.paren);
    switch(inner.N_type){
    case INTEGER:
      return int_type(inner.integer.parser);
    case WRAP:
      return typedef_type(*inner.wrap.parser,name,post);
    case OPTIONAL:
      return (boost::format("%s*") % typedef_type(*inner.optional,name,NULL)).str();
    case NAME:
      return mk_str(inner.name.name);
    case REF:
      return (boost::format("%s *") % mk_str(inner.name.name)).str();
    case FIXEDARRAY:
      if(post){
        *post = (boost::format("[%s]%s") % intconstant_value(inner.fixedarray.length) % *post).str(); 
        return typedef_type(*inner.fixedarray.inner,name,post);
      } 
      else
        return (boost::format("%s *") % typedef_type(*inner.fixedarray.inner,name,NULL)).str();
    case NUNION:
      /*   FOREACH(parser,inner.UNION){
           if(!type_equal(inner.UNION.elem[0],parser))
           throw std::runtime_error("Grammar invalid at UNION");
           } */
      return typedef_type(*inner.nunion.elem[0],name,post);
      //  return (boost::format("%s[%s]") %typedef_type(*inner.fixedarray.inner,name)% intconstant_value(inner.fixedarray.length)).str();
    case STRUCTURE:
    case ARRAY:
    case CHOICE:
    case SELECTP:
      return (boost::format("struct %s") % name).str();
    case APPLY:
      return typedef_type(*inner.apply.inner,name, post);
    default:
      assert(0);
      return 0;
    }
  }
//Negative of a constraint. 
void constraint(std::ostream &out,std::string val, constraintelem &e){
  switch(e.N_type){
  case INTVALUE:
    out << val << "!="<< intconstant_value(e.intvalue);
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
void constraint(std::ostream &out,std::string val,  intconstraint &c){
  switch(c.N_type){
  case SINGLE:
    constraint(out,val,c.single);
    break;
  case SET:
    {
      int first = 0;
      FOREACH(allowed,c.set){
        if(first++ != 0)
          out << " && ";
        constraint(out,val,*allowed);
      }
    }
    break;
  case NEGATE:
    out << "!(";
    constraint(out,val,*c.negate);
    out << ")";
    break;
  }
}
std::string parameter_template(const definition &def,Scope &scope){
  std::stringstream params;
  params << "typename strt_current";
  if(def.parser.parameters){
        FOREACH(param, *def.parser.parameters){
          if(DSTREAM != param->N_type)
            continue;
          params << ",typename strt_"<<mk_str(param->dstream);
        }
  }
  return params.str();
}
std::string parameter_definition(const definition &def, Scope &scope){
  std::stringstream params;
  if(def.parser.parameters){
        FOREACH(param, *def.parser.parameters){
          switch(param->N_type){
          case DSTREAM:
            if(option::templates)
              params << ", strt_"<<mk_str(param->dstream) << " *str_" << mk_str(param->dstream);
            else
              params << ",NailStream *str_" << mk_str(param->dstream);
            scope.add_stream_parameter(mk_str(param->dstream));
            break;
          case DDEPENDENCY:{
            std::string post;
            std::string type = typedef_type(*param->ddependency.type,"",&post);
            std::string name = mk_str(param->ddependency.name);
            assert(post == "");
            params << ","<<  type << "* dep_" << name << post;
            scope.add_dependency_parameter(name,type, 0);
          }
            break;
          }
        }
      }
  return params.str();
}
std::string parameter_invocation(const parameterlist *parameters, const Scope &scope){
  std::stringstream out;
  if(parameters){
    FOREACH(param, *parameters){
      switch(param->N_type){
      case PDEPENDENCY:
        out << "," << scope.dependency_ptr(mk_str(param->pdependency));
        break;
      case PSTREAM:
        out << "," << scope.stream_ptr(mk_str(param->pstream));
        break;
      }
    }
  }
  return out.str();
}
