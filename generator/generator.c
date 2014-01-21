

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "generator.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"
#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
#define p_format "%.*s"
#define p_arg(x) x.count,x.elem

typedef struct expr{
        struct expr *parent;
        const char *str;
        size_t len;
        int pointer;
} expr;

struct expr *expr_new(const char *cstr,size_t length, struct expr *prev,int pointer){
        struct expr *r = malloc(sizeof(expr));
        r->parent = prev;
        r->str = cstr;
        r->len = length;
        r->pointer = pointer;
        return r;
}
struct expr *expr_const(const char *con,expr *parent,int pointerdepth){
        return expr_new(con,strlen(con),parent,pointerdepth);
}
#define expr_narray(array,prev,ptr) expr_new((array).elem,(array).count,prev,ptr)
#define narray_ptr(array,prev) expr_narray(array,prev,1)
#define narray_val(array,prev) expr_narray(array,prev,0)

void expr_print_(struct expr *r,int depth){
        assert(r->pointer>=depth);
        if(r->parent)
        {
                if(r->parent->pointer >= 1){
                        expr_print_(r->parent,1);
                        printf("->");
                }
                else{
                        expr_print_(r->parent,0);
                        printf(".");
                }             
        }
        printf("%.*s",r->len,r->str);
        for(;depth<r->pointer;depth++)
                printf("[0]");
}
void expr_print(struct expr *r){
        expr_print_(r,0);
}
static unsigned long strntoud(unsigned siz,const char *num){
        unsigned i=0;
        unsigned long ret=0;
        for(;i<siz;i++){
                assert(num[i] >= '0' && num[i] <= '9');
                ret= 10 * ret+ num[i] - '0';
        }
        return ret;
}


void write_constparser(constparser_invocation *p){
        switch(p->N_type){
        case CHAR:
        {
                printf("write_bits(8,'%.*s');\n",p_arg(p->CHAR.charcode));
        }
        break;
        case ENDP:
        {
                printf("goto success;");
        }
        break;
        }
}
void write_parser_inner(parser_invocation *p,expr *value,int graceful_error);
void write_parser(parser_invocation *p,expr *value){
        write_parser_inner(p,value,0);
}
void write_parser_inner(parser_invocation *p,expr *value,int graceful_error){
/*TODO*/
        switch(p->N_type){
        case BITS:
        {
                printf("write_bits(%d,",strntoud(p_arg(p->BITS.length)));
                //TODO: print case
                assert(strntoud(p_arg(p->BITS.sign)) == 0);
                expr_print(value);
                printf(");\n");
        }                
                break;
        case UINT:
        {
                printf("write_bits(%d,",strntoud(p_arg(p->UINT.width)));
                //TODO: check that it is 8,16,32,or 64
                expr_print(value);
                printf(");\n");
        }                                
        break;
        case INT_RANGE:
        {
                printf("if(%lu<=",strntoud(p_arg(p->INT_RANGE.lower)));
                expr_print(value);
                printf("&&  %lu>=",strntoud(p_arg(p->INT_RANGE.upper)));
                expr_print(value);
                printf("){");
                write_parser(p->INT_RANGE.inner,value);
                printf("} else \n");
                if(!graceful_error)
                        printf("{return 0;}\n");
        }
        break;
        case CHOICE:
        {
                FOREACH(parser,p->CHOICE.invocations){
                        write_parser_inner(*parser,value,1);
                }
                printf("{return 0;}");
        }
        break;
        default:
                assert(1);
        
        }
}

static int g_num_iters  = 0; 
void emit_parserrule(parserrule *rule,expr *value);
/*ABI: write_bits, save_bits, write_bits_at*/
void emit_field(struct_field *field,expr *value)
{
        emit_parserrule(&field->contents,expr_narray(field->name,value,0));
} 
void emit_const(struct_const *field)
{
        write_constparser(&field->contents); 
}
void emit_STRUCT(struct_rule *rule,expr *value){
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
void emit_REF(ref_rule *ref,expr *value){
        printf("if(!gen_%.*s(out,", ref->name.count, ref->name.elem);
        value->pointer++;
        expr_print(value);
        printf(")){ return 0;}\n");        
}
void emit_EMBED(embed_rule *embed,expr *value){
        printf("if(!gen_%.*s(out,&(", embed->name.count, embed->name.elem);
        expr_print(value);
        printf("))){return 0;}\n");
}
void emit_SCALAR(scalar_rule *scalar,expr *value){
        write_parser(&scalar->parser,value);
}
void emit_OPTIONAL(optional_rule *optional,expr *value)
{
        printf("if(");
        expr_print(value);
        printf("){");
        value->pointer++;
        emit_parserrule(&optional->inner,value);
        printf("}");
}
void emit_CHOICE(choice_rule *choice,expr *value)
{
        printf("switch(");
        expr_print(expr_const("N_TYPE",value,0));
        printf("{");
        FOREACH(option,choice->options){
                printf("case %.*s:\n", p_arg(option->tag));
                emit_parserrule(&option->inner,expr_narray(option->tag,value,0));
        }
        printf("default: return 0;");
        printf("}");
} 
void emit_foreach(parserrule *inner,expr *value){
        char buf[20];
        snprintf(buf,sizeof buf,"iter%d",++g_num_iters);
        printf("FOREACH(%s,",buf);
        expr_print(value);
        printf("){\n");
        emit_parserrule(inner,expr_const(buf,NULL,1));
        printf("}\n");
}
void emit_ARRAY(array_rule *array,expr *value){
//TODO: check for many1 - emit check!. Check for other values too!
        if(strncmp("h_many1",array->parser_combinator.elem,array->parser_combinator.count)){
                printf("if(0==");
                expr_print(expr_const("count",value,0));
                printf("){return 0;}\n");
        }
        emit_foreach(&array->contents,value);
}
void emit_NX_LENGTH(nx_length_rule *length,expr *value){
        write_parser(&length->lengthparser, expr_const("count",value,0)); //Write out dummy
                                                                              //value
        emit_foreach(&length->inner,value);
}
void emit_parserrule(parserrule *rule,expr *value){
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
                    default:
                            assert("boom");
  }
}
FILE *infile(int argc,char **argv){
        char commandbuffer[1024];
        FILE *infile;
        if(argc<2){
                fprintf(stderr,"Usage %s <grammar file>\n", argv[0]);
                exit(-2);
        }
        snprintf(commandbuffer,sizeof commandbuffer,"cpp -I/usr/include/nail/generator < \"%s\" |sed '/^\\#/d'",argv[1]);
        infile = popen(commandbuffer,"r");
    if(!infile){
            fprintf(stderr, "Cannot open pipe\n");
            exit(-1);
    }
    return infile;
}
int main(int argc, char**argv)
{
    uint8_t input[102400];
    size_t inputsize;
    size_t outputsize;
    char *out;
    const struct grammar *result;
    inputsize = fread(input, 1, sizeof(input), infile(argc,argv));
    //fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    //fwrite(input, 1, inputsize, stderr);  
    // print_parser_invocation(parse_parser_invocation(input,inputsize),stdout,0);
    // exit(0);
     result =  parse_grammar(input,inputsize);
     if(result) {
             printf("#include <hammer/hammer.h>\n");
             printf("#include \"%s\"\n",argv[1]);
             printf("#define write_bits(size,val) h_bit_writer_put(out,val,size)\n");
             printf("#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)\n");
            FOREACH(definition,result->rules){
                    printf("int gen_%.*s(HBitWriter* out,struct %.*s *val){\n",definition->name.count,definition->name.elem,definition->name.count,definition->name.elem);
                    emit_parserrule(&definition->rule,expr_const("val",NULL,1));
                    printf("success: return 1;}\n");                    
            }
            return 0;
    } else {
        return 1;
    }
}
