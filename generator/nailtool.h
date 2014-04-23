#ifndef NAILTOOL_H
#define NAILTOOL_H
#include "expr.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
extern "C"{
#include <hammer/hammer.h>
#include <hammer/glue.h>


  extern char parser_template_start[] asm("_binary_parser_template_c_start"); 
  extern char parser_template_end[] asm("_binary_parser_template_c_end");

  extern char header_template_start[] asm("_binary_parser_template_h_start"); 
  extern char header_template_end[] asm("_binary_parser_template_h_end");
}
#include "grammar.h"
typedef struct expr{
  struct expr *parent;
  const char *str;
  size_t len;
  int pointer;
} expr;

#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
extern void emit_generator(std::ostream *out,std::ostream *header,grammar *grammar);
extern void emit_parser(std::ostream *out,grammar *grammar);
extern void emit_header(std::ostream *out,grammar *grammar);
std::string intconstant_value(const intconstant &val);

bool parameter_type_check(parameterlist *param, parameterdefinitionlist *def);

#define mk_str(x) std::string((const char *)(x).elem,(x).count)

struct Dependency{
  std::string name;
  std::string typedef_type;
};
class Scope{ 
  std::map<std::string,Dependency> dependencies;
  std::map<std::string,std::string> streams;
  Scope *parent;

public:
  Scope(Scope *p_parent = NULL) : parent(p_parent) {}
  void add_stream_parameter(std::string value){
    if(!streams.emplace(value, std::string("str_")+value).second){
      throw new std::string("Duplicate stream "+value);
    }
  }
  void add_dependency_parameter(std::string value, std::string type){
    Dependency dep;
    dep.name = std::string("dep_")+value;
    dep.typedef_type = type;
    if(!dependencies.emplace(value,dep).second){
      throw new std::string("Duplicate dependency "+value);
    }
  }
  void add_stream_definition(std::string value){
    if(!streams.emplace(value, std::string("&str_")+value).second){
      throw new std::string("Duplicate dependency "+value);
    }
  }
  void add_dependency_definition(std::string value,std::string type){
    Dependency dep;
    dep.name = std::string("&dep_")+value;
    dep.typedef_type = type;
    if(!dependencies.emplace(value,dep).second){
      throw new std::string("Duplicate dependency "+value);
    }
  }
  std::string dependency_type(std::string name) const{
    auto i = dependencies.find(name);
    if(i==dependencies.end()){
      if(!parent)
        throw std::string("Undefined reference to ") +  name;
      else
        return parent->dependency_type(name);
    }
    return i->second.typedef_type;                
  }
    std::string dependency_ptr(std::string name) const{
    auto i = dependencies.find(name);
    if(i==dependencies.end()){
      if(!parent)
        throw std::string("Undefined reference to ") +  name;
      else
        return parent->dependency_ptr(name);
    }
    return i->second.name;                
  }
  std::string stream_ptr(std::string name) const{
    auto i = streams.find(name);
    if(i==streams.end()){
      if(!parent)
        throw std::string("Undefined reference to ") +  name;
      else
        return parent->stream_ptr(name);
    }
    return i->second;                
  }
};

std::string int_type(const intp &intp );
std::string typedef_type(const parser &p, std::string name, std::string *post);
std::string parameter_definition(const definition &def, Scope &scope );
std::string parameter_invocation(const parameterlist *parameters, const Scope &scope);

void constraint(std::ostream &out,std::string val,  intconstraint &c);

typedef enum {Big, Little} Endian;
#endif
