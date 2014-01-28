#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nailtool.h"
static FILE *outfile = NULL;
#define pr(...) fprintf(stderr,__VA_ARGS__);
static void emit_STRUCT(struct_rule *rule, expr *invalue, expr*outvalue){
        int index = 0;
        pr("{\n"
           "HParsedToken **fields = h_seq_elements("); expr_print(value); pr(");\n");
        pr("assert(%ul == h_seq_len(",rule->fields.count);   expr_print(value);   pr("));\n");
        FOREACH(field,rule->fields){
                if(field->N_TYPE == S_FIELD){
                        emit_parserrule(&field->S_FIELD.contents,expr_arrayidx(index,value),narray_ptr(field->S_FIELD.name));
                }
                else
                        assert(field->N_TYPE == S_CONSTANT);
                index++;
        }
        pr("}")
}
static void emit_ARRAY(array_rule *array, expr *value, expr *outvalue){
        expr *elems= expr_const("elem",outvalue,1);
        expr *count = expr_const("elem",outvalue,0);
        pr("if((");
        expr_print(expr_const("count",outvalue,0));
        pr(" = h_seq_len(");
        expr_print(value);
        pr(")) > 0) {\n");
        expr_print()
        
}
static void emit_parserrule(parserrule *expr, expr *invalue, expr *outvalue){
        switch(rule->N_type){
#define gen(x) case P_ ## x:                        \
                emit_## x (rule->P_##x,invalue,outvalue);   \
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
static void emit_actions(grammar *result){
        FOREACH(definition,grammar->results){
                pr("static void bind_%.*s(const HParseResult *p, const HParsedToken *val,name *out){\n");
                emit_parserrule(expr,expr_const("val",NULL,1));
                pr("}\n");
                pr("HParsedToken *act_%.*s(const HParseResult *p, void *user_data){\n",p_arg(definition->name));
                pr("%.*s * out = H_ALLOC(%.*s);\n", p_arg(definition->name), p_arg(definition->name));
                pr("bind_%.*s(p,p->ast,out);\n",p_arg(definition->name));
                pr("return h_make(p->arena, (HTokenType)TT_##%.*s,out)",definition->name);
                pr("}");
        }
}
void emit_hammer_parser(FILE *outf, grammar *result,const char *header_file){
        outfile = outf;
        pr("#include <hammer/hammer.h>\n");
        pr("#include \"%s\"\n",header_filename);
        emit_actions(result);
        outfile = NULL;
}
