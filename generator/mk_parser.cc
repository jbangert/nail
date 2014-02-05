#include "nailtool.h"
#include <hammer/hammer.h>
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}

#define str(x) std::string((const char *)x.elem,x.count)
extern HAllocator system_allocator;
class GenHammer{
#define ELEM(x) void action(x &param,Expr &in,Expr &outval);    \
  void parser(x &param);                 
  
  ELEM(struct_rule);
  ELEM(nx_length_rule);
  ELEM(array_rule);
  ELEM(choice_rule);
  ELEM(optional_rule);
  ELEM(parserrule);
  ELEM(embed_rule);
  ELEM(ref_rule);
  ELEM(scalar_rule);
  ELEM(wrap_rule);
  int numiter;
  std::ostream &_out;
  std::string header_filename;
  void action(parser_definition &def);
  std::string parser(parser_invocation &parser){
    size_t len;
    const uint8_t* p;
    HBitWriter *bw = h_bit_writer_new(&system_allocator);
    //  if(!gen_parser_invocation(bw,&parser))
      return NULL;
    p = h_bit_writer_get_buffer(bw, &len);
    return std::string((const char*)p,len);
  }
  std::string parser(constparser_invocation &parser){
    size_t len;
    const uint8_t* p;
    HBitWriter *bw = h_bit_writer_new(&system_allocator);
    // if(!gen_constparser_invocation(bw,&parser))
      return NULL;
    p = h_bit_writer_get_buffer(bw, &len);
    return std::string((const char*)p,len);
  }
public:

  GenHammer(std::ostream &parserf , const char *_header_filename ) : _out(parserf),header_filename(_header_filename), numiter(0){}
  void action(grammar *grammar);
};


void GenHammer::parser(scalar_rule &rule){
  _out << parser(rule.parser);
}

void GenHammer::action(scalar_rule &rule, Expr &in, Expr &outval){
  std::string cast;
  if(str(rule.cast) == "UINT")
    cast = "H_CAST_UINT";
  else if(str(rule.cast) == "SINT")
    cast = "H_CAST_SINT";
  else 
    assert("Invalid cast");
  _out<< outval << "="<<cast<< "("<< in<<");\n";
}
void GenHammer::parser(nx_length_rule &rule){
  _out << "h_length_value("<< parser(rule.lengthparser) << ",";
  parser(rule.inner);
  _out << ")";  
}
void GenHammer::action(nx_length_rule &rule, Expr &in, Expr &outval){
  action(rule.inner,in,outval);
}
void GenHammer::parser(embed_rule &rule){
  _out << "hammer_x_" << str(rule.name) << "()";
}
void GenHammer::action(embed_rule &rule, Expr &in, Expr &outval){
  _out<< outval << "= bind_" << str(rule.name) << "();\n";
}
void GenHammer::parser(ref_rule &rule){
  _out <<  "hammer_" << str(rule.name) << "()";
}
void GenHammer::action(ref_rule &rule, Expr &in, Expr &outval){
  outval.make_ptr();
  _out<< outval << "=H_CAST("<< str(rule.name) << ","<<in<<");\n";  
}
void GenHammer::parser(wrap_rule &rule){
  _out << "h_sequence(";
  FOREACH(r,rule.before){
    parser(r->contents);
    _out << ",";    
  }
  parser(rule.inner);
  _out << ",";  
  FOREACH(r,rule.after){
    parser(r->contents);
    _out << ",";    
  }  
  _out << "NULL)";
}
void GenHammer::action(wrap_rule &rule,Expr&in, Expr &outval ){
  HammerSeqElem inval(&in,rule.before.count);
  action(rule.inner,inval,outval);
}
void GenHammer::parser(choice_rule &rule){
  _out << "h_choice(";
  FOREACH(r,rule.options){
    parser(r->inner);
    _out << ",";
  }
  _out << "NULL)";
}
void GenHammer::action(choice_rule &rule, Expr &in, Expr &outval){
  ValExpr typeexpr("N_TYPE",&outval,0);
  ValExpr token_type("token_type",&in,0);
  _out<< typeexpr << "=" << token_type<< ";\n";
  _out<< "switch("<<typeexpr<<"){\n";
  FOREACH(option,rule.options){
    ValExpr outexpr(str(option->tag),&outval,0);
    _out<< "case "<< str(option->tag) << ":\n";
    action(option->inner,in,outexpr);
    _out<< "break;\n";
  }
  _out<< "}";
}
void GenHammer::parser(optional_rule &rule){
  _out << "h_optional(";
  parser(rule.inner);
  _out << ")";
}
void GenHammer::action(optional_rule &rule, Expr &in, Expr &outexpr){
  ValExpr token_type("token_type",&in,0);
  _out<< "if("<<token_type<<"==TT_NONE)";
  _out<< outexpr << "=NULL;";
  _out<< "else {";
  _out<< outexpr << "=h_arena_malloc(p->arena,sizeof(*" << outexpr << "));\n";
  outexpr.make_ptr();
  action(rule,in,outexpr);
  _out<< "}";
}
void GenHammer::parser(array_rule &rule){
  _out << str(rule.parser_combinator) << "(";
  parser(rule.contents);
  _out << ")";
}
void GenHammer::action(array_rule &rule,Expr &in,Expr &outexpr){
  ValExpr elem("elem",&outexpr,0);
  _out << "{int i; HParsedToken **seq = h_seq_elements("<< in<<");\n"
     << ValExpr("count",&outexpr,0) << "= h_seq_len("<<in<<")\n";
  _out << "if(" << ValExpr("count",&outexpr,0) << ">0){\n"
      <<  elem << "=  (void*)h_arena_malloc(p->arena,sizeof(" <<elem<< "[0])*count);\n}\n"
      << "for(i=0;i<"<<ValExpr("count",&outexpr,0)<<";i++){\n";
  ValExpr i("i",NULL,0);
  ArrayElemExpr elemi(&elem,&i);
  ValExpr seqi("seq[i]",NULL,1);
  action(rule.contents,seqi,elemi);
  _out<< "}}";
}
void GenHammer::parser(struct_rule &rule){
  _out << "h_sequence(";
  FOREACH(struct_elem,rule.fields){
    switch(struct_elem->N_type){
      case S_FIELD:
        _out << "h_name(\""<<str(struct_elem->S_FIELD.name)<<"\", ";
        parser(struct_elem->S_FIELD.contents);
        _out <<  "),";
        break;
        case S_CONST:
          parser(struct_elem->S_CONST.contents);
          _out <<  ",";
          break;
    }
  }
  _out<<"NULL)";
}
void GenHammer::action(struct_rule &rule,Expr &in, Expr &out){
  int struct_off = 0;
  FOREACH(struct_elem,rule.fields)
    {
      HammerSeqElem val(&in,struct_off);
      switch(struct_elem->N_type){
      case S_FIELD:
        {
        struct_field &field  = struct_elem->S_FIELD;
        ValExpr outfield(str(field.name), &out,0);
        action(field.contents,val,outfield);
        }
        break;
      case S_CONST:
        
        break;
      }
      struct_off++;
    }
}
void GenHammer::parser(parserrule &rule){ 
  switch(rule.N_type){
#undef ELEM
#define ELEM(x) case P_ ## x:                        \
    parser(*rule.P_##x);                            \
    break;                
    ELEM(STRUCT);
    ELEM(ARRAY);
    ELEM(REF);
    ELEM(EMBED);
    ELEM(SCALAR);
    ELEM(OPTIONAL);
    ELEM(CHOICE);
    ELEM(NX_LENGTH);
    ELEM(WRAP);
  default:
    assert("boom");
    exit(-1);
    break;
  }  
}
void GenHammer::action(parserrule &rule,Expr &in, Expr &out){
      switch(rule.N_type){
#undef ELEM
#define ELEM(x) case P_ ## x:                        \
        action(*rule.P_##x,in,out);                 \
        break;                
        ELEM(STRUCT);
        ELEM(ARRAY);
        ELEM(REF);
        ELEM(EMBED);
        ELEM(SCALAR);
        ELEM(OPTIONAL);
        ELEM(CHOICE);
        ELEM(NX_LENGTH);
        ELEM(WRAP);
      default:
        assert("boom");
        exit(-1);
        break;
      }  
}
void GenHammer::action(parser_definition &def){
  std::string name(str(def.name));
  ValExpr outexpr("out",NULL,1);
  ValExpr in("val",NULL,1);
  _out<< "static void bind_" << name << "(const HParseResult *p, const HParsedToken *val,"<< name << " *out){";
  action(def.rule,in,outexpr);
  _out<< "}\n"
      << "HParsedToken *act_" << name << "(const HParseResult *p, void *user_data) {"
      << name<<" * out= H_ALLOC("<<name<<");"
      <<"  const HParsedToken *val = p->ast;"
      <<"  bind_"<< name <<"(p,val,out);"
      <<"   h_make(p->arena,(HTokenType)TT_ " << name <<",out);}";
}
void GenHammer::action(grammar *grammar)
{
  _out<< "#include <hammer/hammer.h>\n";
  _out<< "#include <hammer/glue.h>\n";
  MAP(action,grammar->rules);
  _out<< std::endl;
}

void emit_hammer_parser(std::ostream *out, grammar *grammar, const char *header){
  assert(out->good());
  GenHammer g(*out,header);
  g.action(grammar);
 
}
