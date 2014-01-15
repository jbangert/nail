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
#define N_CHOICE(inner)                                                 \
        {                                                               \
          __typeof__(out) choice = out;                                 \
          const HParsedToken *choose = (const HParsedToken *)val->user;  \
          choice->N_type = val->token_type;                             \
           {                                                            \
                   const HParsedToken *val= choose;                      \
                   switch(choice->N_type){                              \
                           inner                                        \
                   default:                                             \
                           assert("This parser generator has a bug.");  \
                   }                                                    \
           }                                                            \
        }
#define N_OPTION(name,inner)                                            \
        case name:                                                      \
        {__typeof__(choice->name) * out = &choice->name;                \
                inner}                                                  \
        break;

#define NX_LENGTHVALUE_HACK(lengthp,elemp) N_ARRAY(elemp,h_length_value)

#define N_FIELD(name,inner) {const HParsedToken *val=fields[i]; __typeof__(str->name) *out = &(str->name); inner; i++;}
#define N_DISCARD(inner) i++;
#define N_SCALAR(cast,type,parser) *out = cast(val);
#define N_OPTIONAL(inner) if(val->token_type == TT_NONE) *out=NULL; \
        else{                                                           \
                __typeof__(*out) opt = h_arena_malloc(p->arena,sizeof(**out));          \
                *out = opt;                                             \
                {__typeof__(opt) out = opt;                             \
                inner;                                                  \
                }                                                       \
        }

#define N_PARSER(name) bind_##name(p,val,out);
#define N_DEFPARSER(name,inner)                         \
        static void bind_##name (const HParseResult *p,const HParsedToken *val, name *out) { \
                inner;                                                  \
        }                                                               \
        HParsedToken *act_## name(const HParseResult *p, void *user_data) { \
                name * out= H_ALLOC(name);                              \
                const HParsedToken *val = p->ast;                       \
                bind_## name(p,val,out);                                \
                return h_make(p->arena,(HTokenType)_TT_ ## name,out);    \
        }


