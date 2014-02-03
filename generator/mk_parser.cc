#include "nailtool.h"
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}

#define str(x) std::string((const char *)x.elem,x.count)
class GenHammer{
#define ELEM(x) void gen(x &param,Expr &in,Expr &outval); 
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
  std::ostream &_action;
  std::string header_filename;
  void gen(parser_definition &def);
public:
  GenHammer(std::ostream &_out, const char *_header_filename ) : _action(_out),header_filename(_header_filename), numiter(0){}
  void gen(grammar *grammar);
};
void GenHammer::gen(scalar_rule &rule, Expr &in, Expr &outval){
  std::string cast;
  if(str(rule.cast) == "UINT")
    cast = "H_CAST_UINT";
  else if(str(rule.cast) == "SINT")
    cast = "H_CAST_SINT";
  else 
    assert("Invalid cast");
  _action<< outval << "="<<cast<< "("<< in<<");\n";
}
void GenHammer::gen(nx_length_rule &rule, Expr &in, Expr &outval){
  gen(rule.inner,in,outval);
}
void GenHammer::gen(embed_rule &rule, Expr &in, Expr &outval){
  _action<< outval << "= bind_" << str(rule.name) << "();\n";
}
void GenHammer::gen(ref_rule &rule, Expr &in, Expr &outval){
  outval.make_ptr();
  _action<< outval << "=H_CAST("<< str(rule.name) << ","<<in<<");\n";  
}

void GenHammer::gen(wrap_rule &rule,Expr&in, Expr &outval ){
  HammerSeqElem inval(&in,rule.before.count);
  gen(rule.inner,inval,outval);
}
/*
void GenHammer::gen(ref_rule &rule, Expr &in, Expr &outval){
  _action<< outval << "=" << str(rule.cast) << "("<<in<<")";
}  */
void GenHammer::gen(choice_rule &rule, Expr &in, Expr &outval){
  ValExpr typeexpr("N_TYPE",&outval,0);
  ValExpr token_type("token_type",&in,0);
  _action<< typeexpr << "=" << token_type<< ";\n";
  _action<< "switch("<<typeexpr<<"){\n";
  FOREACH(option,rule.options){
    ValExpr outexpr(str(option->tag),&outval,0);
    _action<< "case "<< str(option->tag) << ":\n";
    gen(option->inner,in,outexpr);
    _action<< "break;\n";
  }
  _action<< "}";
}
void GenHammer::gen(optional_rule &rule, Expr &in, Expr &outexpr){
  ValExpr token_type("token_type",&in,0);
  _action<< "if("<<token_type<<"==TT_NONE)";
  _action<< outexpr << "=NULL;";
  _action<< "else {";
  _action<< outexpr << "=h_arena_malloc(p->arena,sizeof(*" << outexpr << "));\n";
  outexpr.make_ptr();
  gen(rule.inner,in,outexpr);
  _action<< "}";
}
void GenHammer::gen(array_rule &rule,Expr &in,Expr &outexpr){

  ValExpr elem("elem",&outexpr,0);
  _action << "{int i; HParsedToken **seq = h_seq_elements("<< in<<");\n"
     << ValExpr("count",&outexpr,0) << "= h_seq_len("<<in<<")\n";
  _action << "if(" << ValExpr("count",&outexpr,0) << ">0){\n"
      <<  elem << "=  (void*)h_arena_malloc(p->arena,sizeof(" <<elem<< "[0])*count);\n}\n"
      << "for(i=0;i<"<<ValExpr("count",&outexpr,0)<<";i++){\n";
  ValExpr i("i",NULL,0);
  ArrayElemExpr elemi(&elem,&i);
  ValExpr seqi("seq[i]",NULL,1);
  gen(rule.contents,seqi,elemi);
  _action<< "}}";
}

void GenHammer::gen(struct_rule &rule,Expr &in, Expr &out){
  int struct_off = 0;
  FOREACH(struct_elem,rule.fields)
    {
      HammerSeqElem val(&in,struct_off);
      switch(struct_elem->N_type){
      case S_FIELD:
        {
        struct_field &field  = struct_elem->S_FIELD;
        ValExpr outfield(str(field.name), &out,0);
        gen(field.contents,val,outfield);
        }
        break;
      case S_CONST:
        
        break;
      }
      struct_off++;
    }
}
void GenHammer::gen(parserrule &rule,Expr &in, Expr &out){
      switch(rule.N_type){
#undef ELEM
#define ELEM(x) case P_ ## x:                        \
        gen(*rule.P_##x,in,out);                 \
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
void GenHammer::gen(parser_definition &def){
  std::string name(str(def.name));
  ValExpr outexpr("out",NULL,1);
  ValExpr in("val",NULL,1);
  _action<< "static void bind_" << name << "(const HParseResult *p, const HParsedToken *val,"<< name << " *out){";
  gen(def.rule,in,outexpr);
  _action<< "}\n"
      << "HParsedToken *act_" << name << "(const HParseResult *p, void *user_data) {"
      << name<<" * out= H_ALLOC("<<name<<");"
      <<"  const HParsedToken *val = p->ast;"
      <<"  bind_"<< name <<"(p,val,out);"
      <<"   h_make(p->arena,(HTokenType)TT_ " << name <<",out);}";
}
void GenHammer::gen(grammar *grammar)
{
  _action<< "#include <hammer/hammer.h>\n";
  _action<< "#include <hammer/glue.h>\n";
  MAP(gen,grammar->rules);
  _action<< std::endl;
}

void emit_hammer_parser(std::ostream *out, grammar *grammar, const char *header){
  assert(out->good());
  GenHammer g(*out,header);
  g.gen(grammar);
 
}
