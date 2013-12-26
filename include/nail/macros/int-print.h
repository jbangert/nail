/* debug pretty-printing*/
#define N__FORMAT_UINT "%lu"
#define N__FORMAT_SINT "%ld"
#define N__CAST_UINT(x) (unsigned long)(x)
#define N__CAST_SINT(x) (signed long)(x)
#define N__PRINTIND(format,...) fprintf(out,"\n%*s" format ,2*indent,"", ##__VA_ARGS__);

#define N_PARSER(name,inner) void print_ ## name(const name *val, FILE *out,int indent){ \
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

#define N__CAST(cast,x) TOKENPASTE(N__CAST_,cast)(x) 
#define N__FMT(cast) TOKENPASTE(N__FORMAT_,cast)
#define N_SCALAR(cast,type,parser) fprintf(out,N__FMT(cast),N__CAST(cast,*val))


