#include "nailtool.h"

#define p_format "%.*s"
#define p_arg(x) x.count,(const char*)x.elem

#define str(x) std::string((const char *)x.elem,x.count)
#define printf(...)  ERROR

static unsigned long strntoud(unsigned siz,const char *num){
        unsigned i=0;
        unsigned long ret=0;
        for(;i<siz;i++){
                assert(num[i] >= '0' && num[i] <= '9');
                ret= 10 * ret+ num[i] - '0';
        }
        return ret;
}
class GenGenerator{
  std::ostream &out;
  const char *header_filename;
  int num_iters;
  void write_constparser(constparser_invocation*p);
  void write_parser_inner(parser_invocation *p,Expr &value,int graceful_error);
  void write_parser(parser_invocation *p,Expr &value); 
  void emit_parserrule(parserrule *rule, Expr &value);
  void emit_field(struct_field *field,Expr &value);
  void emit_const(struct_const *field);
  void emit_STRUCT(struct_rule *rule,Expr& value);
  void emit_REF(ref_rule *rule,Expr& value);
  void emit_EMBED(embed_rule *rule,Expr& value);
  void emit_SCALAR(scalar_rule *rule,Expr& value);
  void emit_OPTIONAL(optional_rule *rule,Expr& value);
  void emit_CHOICE(choice_rule *rule,Expr& value);
  void emit_foreach(parserrule *inner,Expr& value);
  void emit_ARRAY(array_rule *array,Expr& value);
  void emit_NX_LENGTH(nx_length_rule *length,Expr& value);
  void emit_WRAP(wrap_rule *rule, Expr& value);
public:
  GenGenerator(std::ostream &_out, const char *_header_filename ) : out(_out),header_filename(_header_filename), num_iters(0){ 
  }
  void emit_grammar( grammar *gramm);
};
void GenGenerator::write_constparser(constparser_invocation *p){
        switch(p->N_type){
        case CHAR:
        {
          out<< "write_bits(8,'" << str(p->CHAR.charcode) << ");\n";
        }
        break;
        case ENDP:
        {
          out << "goto success;\n";
        }
        break;
        }
}
void GenGenerator::write_parser(parser_invocation *p,Expr &value){
        write_parser_inner(p,value,0);
}
void GenGenerator::write_parser_inner(parser_invocation *p,Expr &value,int graceful_error){
/*TODO*/
        switch(p->N_type){
        case BITS:
          assert(strntoud(p_arg(p->BITS.sign)) == 0);
          out << "write_bits(" << strntoud(p_arg(p->BITS.length)) << value << ");\n";
          break;
        case UINT:
  //TODO: check that it is 8,16,32,or 64
          out << "write_bits("<< strntoud(p_arg(p->UINT.width)) << value  <<");\n";
          break;
        case INT_RANGE:
          out<< "if(" << strntoud(p_arg(p->INT_RANGE.lower))<< "<= "<< value ;
          out << "&& " << value << strntoud(p_arg(p->INT_RANGE.upper)) << "){";
          write_parser(p->INT_RANGE.inner,value);
          out << "} else \n";
          if(!graceful_error)
            out << "{return 0;}\n";
          break;
        case CHOICE:
          FOREACH(parser,p->CHOICE.invocations){
            write_parser_inner(*parser,value,1);
          }
          out<<"{return 0;}";
          break;
        default:
                assert(1);
        }
}
/*ABI: write_bits, save_bits, write_bits_at*/
void GenGenerator::emit_field(struct_field *field,Expr &value)
{
  ValExpr name(str(field->name),&value,0);
  emit_parserrule(&field->contents,name);
} 
void GenGenerator::emit_const(struct_const *field)
{
        write_constparser(&field->contents); 
}
void GenGenerator::emit_STRUCT(struct_rule *rule,Expr &value){
        FOREACH(struct_elem,rule->fields)
        {
                switch(struct_elem->N_type){
                case S_FIELD:
                        emit_field(&struct_elem->S_FIELD,value);
                        break;
                case S_CONST:
                        emit_const(&struct_elem->S_CONST);
                        break;
                }
        }
}
void GenGenerator::emit_REF(ref_rule *ref,Expr &value){
  out<< "if(!gen_" << str(ref->name) << "(out,";
  value.make_ptr();
  out<< value << ")){ return 0;}\n";        
}
void GenGenerator::emit_EMBED(embed_rule *embed,Expr &value){
  out << "if(!gen_%.*s(out,&(" <<str(embed->name) << value << "))){return 0;}\n";
}
void GenGenerator::emit_SCALAR(scalar_rule *scalar,Expr &value){
  write_parser(&scalar->parser,value);
}
void GenGenerator::emit_OPTIONAL(optional_rule *optional,Expr &value)
{
  out << "if(" << value << "){";
  value.make_ptr();
  emit_parserrule(&optional->inner,value);
  out << "}";
}
void GenGenerator::emit_CHOICE(choice_rule *choice,Expr &value)
{
  out << "switch(" << ValExpr("N_TYPE",&value,0) << "){\n";
  FOREACH(option,choice->options){
    ValExpr name(str(option->tag),&value,0);
    out << "case " << str(option->tag) << ":\n";
    emit_parserrule(&option->inner,name);
  }
  out<< "default: return 0;\n }\n";
} 
void GenGenerator::emit_foreach(parserrule *inner,Expr &value){
        char buf[20];
        ValExpr itername(buf,NULL,1);
        snprintf(buf,sizeof buf,"iter%d",++num_iters);
        out<<"FOREACH("<<buf<<value<<"){\n";
        emit_parserrule(inner,itername);
        out<<"}\n";
}
void GenGenerator::emit_ARRAY(array_rule *array,Expr &value){
//TODO: check for many1 - emit check!. Check for other values too!
  if(!strncmp("h_many1",(char *)array->parser_combinator.elem,array->parser_combinator.count)){
          out<<"if(0=="<<ValExpr("count",&value,0)<<"){return 0;}\n";
        }
        emit_foreach(&array->contents,value);
}
void GenGenerator::emit_NX_LENGTH(nx_length_rule *length,Expr &value){
  ValExpr lengthfield("count",&value,0);
        write_parser(&length->lengthparser, lengthfield); //Write out dummy
                                                                              //value
        emit_foreach(&length->inner,value);
}
void GenGenerator::emit_WRAP(wrap_rule *rule,Expr &value){
        FOREACH(cnst, rule->before){
                emit_const(cnst);
        }
        emit_parserrule(&rule->inner,value);
        FOREACH(cnst, rule->after){
                emit_const(cnst);
        }
        

}
void GenGenerator::emit_parserrule(parserrule *rule,Expr &value){
        switch(rule->N_type){
#define gen(x) case P_ ## x:                        \
                emit_## x (rule->P_##x,value);      \
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
  }
}
void GenGenerator::emit_grammar( grammar *grammar)
{
  out << "#include <hammer/hammer.h>\n";
  out<<"#include \"" << header_filename << "\"\n";
  out<<"#define write_bits(size,val) h_bit_writer_put(out,val,size)\n";
  out<<"#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)\n";
  FOREACH(definition,grammar->rules){
    ValExpr outfield("val",NULL,1);
    out<<"int gen_"<<str(definition->name)<<"(HBitWriter* out,struct"<<str(definition->name)<<"  *val){\n";
    emit_parserrule(&definition->rule,outfield);
    out<<"success: return 1;}\n"<< std::endl;                    
  }

}
void emit_generator(std::ostream *out, grammar *grammar, const char *header){
  GenGenerator g(*out,header);
  g.emit_grammar(grammar);
}
