/* debug pretty-printing*/
#define N__FORMAT_UINT "%lu"
#define N__FORMAT_SINT "%ld"
#define N__CAST_UINT(x) (unsigned long)(x)
#define N__CAST_SINT(x) (signed long)(x)
#define N__PRINTIND(format,...) fprintf(out,"\n%*s" format ,2*indent,"", ##__VA_ARGS__);
#define N_PARSER(name) print_##name(val,out,indent)
#define N_DEFPARSER(name,inner) void print_ ## name(const name *val, FILE *out,int indent){ \
                N__PRINTIND("{\"%s\":",#name);indent++;                     \
                inner; indent--; \
                N__PRINTIND("}\n");}                                      
                
#define N_STRUCT(inner) {                       \
        __typeof__(val) str = val;              \
        N__PRINTIND("{");                             \
        indent++;                               \
        inner;                                  \
        indent--;                               \
        N__PRINTIND("}");                             \
        }
#define N_WRAP(before,inner,after) {            \
                inner;                          \
        }
#define N_FIELD(name,inner) {                   \
        typeof(str->name) *val = &str->name;    \
        N__PRINTIND("\"%s\":",#name);            \
        inner;                                  \
        fprintf(out,",");                      \
        }
#define N_ARRAY(inner,combinator){                              \
        __typeof__(val) arr = val;                              \
        int i;                                                  \
        N__PRINTIND("[")                                           \
        for(i=0;i<arr->count;i++){                              \
                __typeof__(arr->elem[i]) *val = &arr->elem[i];  \
                inner;                                          \
                if(i+1<arr->count)                              \
                        fprintf(out,",");                       \
        }                                                       \
        N__PRINTIND("]")                                           \
        }
#undef NX_STRING
#define NX_STRING(inner,combinator){            \
                fprintf(out,"\"%.*s\"",val->count,val->elem);   \
                        }
#define N_CHOICE(inner){                                                \
        __typeof__(val) choice = val;                                   \
        switch(choice->N_type){                                            \
        inner                                                           \
        default:                                                        \
                assert("We have a bug");                                \
                        }                                               \
        }

#define N_OPTION(name,inner)                                            \
        case name:{                                                     \
                __typeof__(choice->name) *val = &choice->name;          \
                N__PRINTIND("(%s)->",#name);                            \
                inner;                                                  \
        }                                                               \
        break;
#define NX_LENGTHVALUE_HACK(lenp,elemp) N_ARRAY(elemp,h_length_value)
#define N_OPTIONAL(inner) {                             \
        __typeof__(*val) opt = *val;                      \
                if(opt){                                \
                        __typeof__(opt) val = opt;    \
                        inner;                          \
                }                                       \
                else {                                  \
                        fprintf(out,"(null)");          \
                }                                       \
        }
#define N_REF(inner) N_OPTIONAL(N_PARSER(inner))
#define N_CONSTANT(inner)
#define N__CAST(cast,x) TOKENPASTE(N__CAST_,cast)(x) 
#define N__FMT(cast) TOKENPASTE(N__FORMAT_,cast)
#define N_SCALAR(cast,type,parser) fprintf(out,N__FMT(cast),N__CAST(cast,*val))
