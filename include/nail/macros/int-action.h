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
#define N_FIELD(name,inner) {const HParsedToken *val=fields[i]; __typeof__(str->name) *out = &(str->name); inner; i++;}
#define N_SCALAR(cast,type,parser) *out = cast(val);
#define N_PARSER(name,inner) HParsedToken *act_## name(const HParseResult *p, void *user_data) { \
                name * out= H_ALLOC(name);                              \
                const HParsedToken *val = p->ast;                             \
                inner                                                   \
                        return h_make(p->arena,(HTokenType)TT_ ## name,out); \
        }


