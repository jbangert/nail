

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "generator.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"
#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
#define p_format "%.*s"
#define p_arg(x) x.count,x.elem
typedef struct rope{
  struct rope *prev;
  const char *str;
  size_t len;
} rope;
struct rope *rope_new(const char *cstr,size_t length, struct rope *prev){
        struct rope *r = malloc(sizeof(rope));
        r->prev = prev;
        r->str = cstr;
        r->len = length;
        return r;
}
struct rope *rope_const (const char*cstr, struct rope *prev ){
        return rope_new(cstr,strlen(cstr),prev);
}
#define rope_narray(array,prev) rope_new((array).elem,(array).count,prev)
void rope_print(struct rope *r){
        if(r->prev)
                rope_print(r->prev);
        printf("%.*s",r->len,r->str);
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
        }
}
void write_parser_inner(parser_invocation *p,rope *value,int graceful_error);
void write_parser(parser_invocation *p,rope *value){
        write_parser_inner(p,value,0);
}
void write_parser_inner(parser_invocation *p,rope *value,int graceful_error){
/*TODO*/
        switch(p->N_type){
        case BITS:
        {
                printf("write_bits(%d,",strntoud(p_arg(p->BITS.length)));
                //TODO: print case
                assert(strntoud(p_arg(p->BITS.sign)) == 0);
                rope_print(value);
                printf(");\n");
        }                
                break;
        case UINT:
        {
                printf("write_bits(%d,",strntoud(p_arg(p->UINT.width)));
                //TODO: check that it is 8,16,32,or 64
                rope_print(value);
                printf(");\n");
        }                                
        break;
        case INT_RANGE:
        {
                printf("if(%lu<",strntoud(p_arg(p->INT_RANGE.lower)));
                rope_print(value);
                printf("&&  %lu>",strntoud(p_arg(p->INT_RANGE.upper)));
                rope_print(value);
                printf("){");
                write_parser(p->INT_RANGE.inner,value);
                printf("} else \n");
                if(!graceful_error)
                        printf("{return NULL;}\n");
        }
        break;
        case CHOICE:
        {
                FOREACH(parser,p->CHOICE.invocations){
                        write_parser_inner(*parser,value,1);
                }
                printf("{return NULL;}");
        }
        break;
        default:
                assert(1);
        
        }
}

static int g_num_iters  = 0; 
void emit_parserrule(parserrule *rule,rope *value);
/*ABI: write_bits, save_bits, write_bits_at*/
void emit_field(struct_field *field,rope *value)
{
        emit_parserrule(&field->contents,rope_narray(field->name,rope_const("->",value)));
} 
void emit_const(struct_const *field)
{
        write_constparser(&field->contents); 
}
void emit_STRUCT(struct_rule *rule,rope *value){
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
void emit_REF(ref_rule *ref,rope *value){
        printf("if(!gen_%.*s(out,*(", ref->name.count, ref->name.elem);
        rope_print(value);
        printf("))){ return NULL;}\n");        
}
void emit_EMBED(embed_rule *embed,rope *value){
        printf("if(!gen_%.*s(out,", embed->name.count, embed->name.elem);
        rope_print(value);
        printf(")){return NULL;}\n");
}
void emit_SCALAR(scalar_rule *scalar,rope *value){
        write_parser(&scalar->parser,value);
}
void emit_OPTIONAL(optional_rule *optional,rope *value)
{
        printf("if(");
        rope_print(value);
        printf("){");
        emit_parserrule(&optional->inner,rope_const("[0]",value));
        printf("}");
}
void emit_CHOICE(choice_rule *choice,rope *value)
{
        printf("switch(");
        rope_print(value);
        printf(".N_TYPE){");
        FOREACH(option,choice->options){
                printf("case %.*s:\n", p_arg(option->tag));
                emit_parserrule(&option->inner,rope_narray(option->tag,rope_const("->",value)));
        }
        printf("default: return NULL;");
        printf("}");
} 
void emit_foreach(parserrule *inner,rope *value){
        char buf[20];
        snprintf(buf,sizeof buf,"iter%d",++g_num_iters);
        printf("FOREACH(%s,",buf);
        rope_print(value);
        printf("){\n");
        emit_parserrule(inner,rope_const(buf,NULL));
        printf("}\n");
}
void emit_ARRAY(array_rule *array,rope *value){
//TODO: check for many1 - emit check!. Check for other values too!
        if(strncmp("h_many1",array->parser_combinator.elem,array->parser_combinator.count)){
                printf("if(0==");
                rope_print(rope_const(".count)",value));
                printf("{return NULL;}\n");
        }
        emit_foreach(&array->contents,value);
}
void emit_NX_LENGTH(nx_length_rule *length,rope *value){
        write_parser(&length->lengthparser, rope_const(".count",value)); //Write out dummy
                                                                              //value
        emit_foreach(&length->inner,value);
}
void emit_parserrule(parserrule *rule,rope *value){
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
             printf("#define write_bits(size,val) h_bit_writer_put(out,val,size)");
            FOREACH(definition,result->rules){
                    printf("char * gen_%.*s(HBitWriter* out,%.*s *val){\n",definition->name.count,definition->name.elem,definition->name.count,definition->name.elem);
                    emit_parserrule(&definition->rule,rope_const("val",NULL));
                    printf("}\n");                    
            }
            return 0;
    } else {
        return 1;
    }
}
