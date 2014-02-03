#include "nailtool.h"
#define MAP(f,collection) FOREACH(iter,collection){ f(*iter);}

#define str(x) std::string((const char *)x.elem,x.count)
class GenHammer{
#define ELEM(x) void action(x &param,Expr &in,Expr &outval); 
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
  std::ostream &out;
  std::string header_filename;
  void action(parser_definition &def);
public:
  GenHammer(std::ostream &_out, const char *_header_filename ) : out(_out),header_filename(_header_filename), numiter(0){}
  void action(grammar *grammar);
};
void GenHammer::action(scalar_rule &rule, Expr &in, Expr &outval){
  std::string cast;
  if(str(rule.cast) == "UINT")
    cast = "H_CAST_UINT";
  else if(str(rule.cast) == "SINT")
    cast = "H_CAST_SINT";
  else 
    assert("Invalid cast");
  out << outval << "="<<cast<< "("<< in<<");\n";
}
void GenHammer::action(nx_length_rule &rule, Expr &in, Expr &outval){
  action(rule.inner,in,outval);
}
void GenHammer::action(embed_rule &rule, Expr &in, Expr &outval){
  out << outval << "= bind_" << str(rule.name) << "();\n";
}
void GenHammer::action(ref_rule &rule, Expr &in, Expr &outval){
  outval.make_ptr();
  out << outval << "=H_CAST("<< str(rule.name) << ","<<in<<");\n";  
}

void GenHammer::action(wrap_rule &rule,Expr&in, Expr &outval ){
  HammerSeqElem inval(&in,rule.before.count);
  action(rule.inner,inval,outval);
}
/*
void GenHammer::action(ref_rule &rule, Expr &in, Expr &outval){
  out << outval << "=" << str(rule.cast) << "("<<in<<")";
}  */
void GenHammer::action(choice_rule &rule, Expr &in, Expr &outval){
  ValExpr typeexpr("N_TYPE",&outval,0);
  ValExpr token_type("token_type",&in,0);
  out << typeexpr << "=" << token_type<< ";\n";
  out << "switch("<<typeexpr<<"){\n";
  FOREACH(option,rule.options){
    ValExpr outexpr(str(option->tag),&outval,0);
    out << "case "<< str(option->tag) << ":\n";
    action(option->inner,in,outexpr);
    out << "break;\n";
  }
  out << "}";
}
void GenHammer::action(optional_rule &rule, Expr &in, Expr &outexpr){
  ValExpr token_type("token_type",&in,0);
  out << "if("<<token_type<<"==TT_NONE)";
  out << outexpr << "=NULL;";
  out << "else {";
  out << outexpr << "=h_arena_malloc(p->arena,sizeof(*" << outexpr << "));\n";
  outexpr.make_ptr();
  action(rule.inner,in,outexpr);
  out << "}";
}
void GenHammer::action(array_rule &rule,Expr &in,Expr &outexpr){

  ValExpr elem("elem",&outexpr,0);
  out<< "{int i; HParsedToken **seq = h_seq_elements("<< in<<");\n"
     << ValExpr("count",&outexpr,0) << "= h_seq_len("<<in<<")\n";
  out << "if(" << ValExpr("count",&outexpr,0) << ">0){\n"
      <<  elem << "=  (void*)h_arena_malloc(p->arena,sizeof(" <<elem<< "[0])*count);\n}\n"
      << "for(i=0;i<"<<ValExpr("count",&outexpr,0)<<";i++){\n";
  ValExpr i("i",NULL,0);
  ArrayElemExpr elemi(&elem,&i);
  ValExpr seqi("seq[i]",NULL,1);
  action(rule.contents,seqi,elemi);
  out << "}}";
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
void GenHammer::action(parserrule &rule,Expr &in, Expr &out){
      switch(rule.N_type){
#define gen(x) case P_ ## x:                        \
        action(*rule.P_##x,in,out);                 \
        break;                
        gen(STRUCT);
        gen(ARRAY);
        gen(REF);
        gen(EMBED);
        gen(SCALAR);
        gen(OPTIONAL);
        gen(CHOICE);
        gen(NX_LENGTH);
        gen(WRAP);
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
  out << "static void bind_" << name << "(const HParseResult *p, const HParsedToken *val,"<< name << " *out){";
  action(def.rule,in,outexpr);
  out << "}\n"
      << "HParsedToken *act_" << name << "(const HParseResult *p, void *user_data) {"
      << name<<" * out= H_ALLOC("<<name<<");"
      <<"  const HParsedToken *val = p->ast;"
      <<"  bind_"<< name <<"(p,val,out);"
      <<"   h_make(p->arena,(HTokenType)TT_ " << name <<",out);}";
}
void GenHammer::action(grammar *grammar)
{
  out << "#include <hammer/hammer.h>\n";
  out << "#include <hammer/glue.h>\n";
  MAP(action,grammar->rules);
  out << std::endl;
}

void emit_hammer_parser(std::ostream *out, grammar *grammar, const char *header){
  assert(out->good());
  GenHammer g(*out,header);
  g.action(grammar);
 
}
