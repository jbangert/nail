#ifndef NAILTOOL_H
#define NAILTOOL_H
#include "expr.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <map>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

namespace option {
        extern bool templates;
        extern bool cpp;
}
extern unsigned char parser_template_cc[];
extern unsigned char parser_template_c[];
extern unsigned char parser_template_h[];
extern unsigned int parser_template_c_len, parser_template_cc_len, parser_template_h_len;
#include "grammar.h"
typedef struct expr{
  struct expr *parent;
  const char *str;
  size_t len;
  int pointer;
} expr;

#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
extern void emit_generator(std::ostream *out,std::ostream *header,grammar *grammar);
extern void emit_directparser(std::ostream *out, std::ostream *header, grammar *grammar);
extern void emit_parser(std::ostream *out,std::ostream *header,grammar *grammar);
extern void emit_header(std::ostream *out,grammar *grammar);
std::string intconstant_value(const intconstant &val);

bool parameter_type_check(parameterlist *param, parameterdefinitionlist *def);

#define mk_str(x) std::string((const char *)(x).elem,(x).count)
struct NailException : public std::logic_error { 
public:
NailException(const std::string &message) : std::logic_error(message){}              
};
struct Dependency{
  std::string name;
  std::string typedef_type;
  uint64_t width; // XXX: Replace by a line that should be inserted to output this Dependency
};
class Scope{ 
  std::map<std::string,Dependency> dependencies;
  std::map<std::string,std::string> streams;
  Scope *parent;

public:
  Scope(Scope *p_parent = NULL) : parent(p_parent) {}
  void add_stream_parameter(std::string value){
    if(!streams.emplace(value, std::string("str_")+value).second){
      throw NailException("Duplicate stream "+value);
    }
  }
  void add_dependency_parameter(std::string value, std::string type,uint64_t width){
    Dependency dep;
    dep.name = std::string("dep_")+value;
    dep.typedef_type = type;
    dep.width = width;
    if(!dependencies.emplace(value,dep).second){
      throw NailException("Duplicate dependency "+value);
    }
  }
  void add_stream_definition(std::string value){
    if(!streams.emplace(value, std::string("&str_")+value).second){
      throw NailException("Duplicate dependency "+value);
    }
  }
  void add_dependency_definition(std::string value,std::string type,uint64_t width ){
    Dependency dep;
    dep.name = std::string("&dep_")+value;
    dep.typedef_type = type;
    dep.width = width;
    if(!dependencies.emplace(value,dep).second){
      throw NailException("Duplicate dependency "+value);
    }
  }
  std::string dependency_type(std::string name) const{
    auto i = dependencies.find(name);
    if(i==dependencies.end()){
      if(!parent)
              throw NailException(std::string("Undefined reference to ") +  name);
      else
        return parent->dependency_type(name);
    }
    return i->second.typedef_type;                
  }
    std::string dependency_ptr(std::string name) const{
    auto i = dependencies.find(name);
    if(i==dependencies.end()){
      if(!parent)
              throw NailException(std::string("Undefined reference to ") +  name);
      else
        return parent->dependency_ptr(name);
    }
    return i->second.name;                
  }
  std::string stream_ptr(std::string name) const{
    auto i = streams.find(name);
    if(i==streams.end()){
      if(!parent)
              throw NailException(std::string("Undefined reference to ") +  name);
      else
        return parent->stream_ptr(name);
    }
    return i->second;                
  }
  bool is_local_dependency(std::string name){
    return dependencies.count(name)>0;
  }
  uint64_t dependency_width(std::string name){
    auto i = dependencies.find(name);
    if(i==dependencies.end()){
      if(!parent)
              throw NailException(std::string("Undefined reference to ") +  name);
      else
        return parent->dependency_width(name);
    }
    return i->second.width;            
  }
};

std::string int_type(const intp &intp );
std::string typedef_type(const parser &p, std::string name, std::string *post);
std::string parameter_definition(const definition &def, Scope &scope );
std::string parameter_invocation(const parameterlist *parameters, const Scope &scope);
std::string parameter_template(const definition &def, Scope &scope);
void constraint(std::ostream &out,std::string val,  intconstraint &c);

typedef enum {Big, Little} Endian;
#endif
