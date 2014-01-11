#define UINT(x) H_CAST_UINT(x)
#define SINT(x) H_CAST_SINT(x)
#define N_STRUCT(inner)                                           \
        {                                                           \
                __typeof__(out) str =  out;                           \
                int i =0;                                             \
                const HParsedToken **fields = h_seq_elements(val);    \
                inner                                                 \
        }


#define  N_ARRAY(inner,combinator)                                         \
        {                                                               \
           __typeof__(out) arr =out;                                        \
           int i;                                                       \
           const HParsedToken **seq = h_seq_elements(val);                    \
           out->count = h_seq_len(val);                                 \
           if(out->count >0)                                            \
                   out->elem = (void *)h_arena_malloc(p->arena,sizeof(out->elem[0])*out->count); \
           for(i=0;i<out->count;i++){                    \
             const HParsedToken *val = seq[i];                \
             __typeof__(arr->elem[i]) *out = &arr->elem[i];   \
             inner                                     \
           }                                            \
        }

#define NX_LENGTHVALUE_HACK(lengthp,elemp) {    \
        fpos_t lengthpos= ftell(out);               \
        fpos_t datapos,endpos;
        {                                       \
                size_t val =0;                  \
                lengthp;                        \
        }                                       \
        datapos = ftell(out);                   \



}

#define N_FIELD(name,inner) {const typeof(val->name) *field = &val->name; {const typeof(field) val = field; inner}}
// #define N_DISCARD(inner) inner;
#define N_SCALAR(cast,type,parser) parser(val)
#define N_OPTIONAL(inner) if(val){inner;}
#define N_PARSER(name) print_##name(out,val);
#define N_DEFPARSER(name,inner)                                         \
        char * print_##name (const name *val) {    \
                inner;                                                  \
        }                                                               \
        
