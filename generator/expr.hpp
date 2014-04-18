#ifndef EXPR_H
#define EXPR_H
#include <stdarg.h>
#include <iostream>
#include <cassert>
#include <string>
struct Expr{
public:
  virtual ~Expr(){}
  virtual int is_ptr() const = 0;
  //  virtual void make_ptr() = 0;
  virtual void print_val(std::ostream &str) const = 0;
  virtual void print_ptr(std::ostream &str) const = 0;
  friend std::ostream & operator<<(std::ostream &stream, const Expr &expr){
    expr.print_val(stream);
    return stream;
  }  
};
class ValExpr : public Expr{
  const std::string _val;
  const Expr *_parent;
  int _ptr_depth;
  void print_inner(std::ostream &i, int depth) const{
    assert(_ptr_depth >= depth);
    if(_parent){
      if(_parent->is_ptr()){
        _parent->print_ptr(i);
        i<<"->";
      }
      else{
        _parent->print_val(i);
        i<<".";
      }
    }
    i << _val;
    for(;depth<_ptr_depth;depth++)
      i<<"[0]";
  }
protected:
  virtual void print_val(std::ostream &str) const{
    print_inner(str,0);  
  }
  virtual void print_ptr(std::ostream &str) const{
    print_inner(str,1);
  }
public: 
  virtual int is_ptr() const{
    return _ptr_depth ;
  }
  virtual void make_ptr() {
    assert(false);
    //    exit(-1);
  }
  ValExpr(const std::string val, const Expr *parent = NULL, int pointer_depth = 0) : _val(val), _ptr_depth(pointer_depth), _parent(parent){}
};
class DerefExpr : public Expr{
  const Expr &par;
public: 
  DerefExpr(const Expr &parent): par(parent){}
  virtual int is_ptr() const{
    int i = par.is_ptr();
    if(!i) return 0;
    else return i-1;
  }
  virtual void make_ptr(){
    assert(false);
    //    exit(-1);
  }
  virtual void print_val(std::ostream &str) const{
    print_ptr(str);
    str << "[0]";
  }
  virtual void print_ptr(std::ostream &str) const{
    par.print_val(str);
  }
  
};
class ArrayElemExpr : public Expr{
  const Expr *_parent;
  const Expr *_index;
  int free_index;
public:
  ArrayElemExpr(const Expr *parent, const Expr *index): _parent(parent), _index(index),free_index(0){}
  ArrayElemExpr(const Expr *parent,int elem): _parent(parent), _index(new ValExpr(std::to_string(elem),NULL,0)), free_index(1){} 
  ~ArrayElemExpr(){
    if(free_index)
      delete _index;
  }
  virtual int is_ptr() const {
    return 0;
  }
  virtual void print_val(std::ostream &str) const{
    str << *_parent << "[" << *_index << "]";
  }
  virtual void print_ptr(std::ostream &s) const{
    assert("false");
  }              
  virtual void make_ptr() { 
    assert("false");
  }
};
class HammerSeqElem : public Expr {
  const Expr *_parent;
  const Expr *_index;
  int free_index;
public:
  HammerSeqElem(const Expr *parent, const Expr *index): _parent(parent), _index(index),free_index(0){}
  HammerSeqElem(const Expr *parent,int elem): _parent(parent), _index(new ValExpr(std::to_string(elem),NULL,0)), free_index(1){} 
  ~HammerSeqElem(){
    if(free_index)
      delete _index;
  }
  virtual int is_ptr() const {
    return 0;
  }
  virtual void print_val(std::ostream &str) const{
    str << "h_seq_index("<< *_parent << "," << *_index << ")";
  }
  virtual void print_ptr(std::ostream &s) const{
    assert("false");
  }              
  virtual void make_ptr() { 
    assert("false");
  }
};


#endif
