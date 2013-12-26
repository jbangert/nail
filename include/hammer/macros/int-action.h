#define HM_F(cast,type,field,parser) ret->field = HM_DO_CAST(cast,type,fields[i]); i++;
#define HM_ARRAY(cast,type,field,parser) ret->field.count = h_seq_len(fields[i]); \
        do{int j; HParsedToken **seq = h_seq_elements(fields[i]); \
                ret->field.elem = (HM_DO_TYPE(cast,type) *)h_arena_malloc(p->arena, sizeof(HM_DO_TYPE(cast,type)) * ret->field.count); /*  WARNING, can fail*/ \
        for(j=0;j<ret->field.count;j++){ret->field.elem[j] = HM_DO_CAST(cast,type,seq[j]); } \
        }while(0); i++;
#define HM_OPTIONAL(cast,type,field,parser,default) if(fields[i]->token_type == TT_NONE){ret->field = default;i++;} else {HM_F(cast,type,field,parser)}
//#define HM_F_ARRAY(type,field,parser)  ret->field = /* We want an array of type */
// HM__TO(H_CAST_SEQ ,type,field,parser)


#define HM_STRUCT_SEQ_IMPL(name,...)                                     \
        HParsedToken * TOKENPASTE2(act_,HM_NAME) (const HParseResult *p, void* user_data) \
        {                                                               \
          int i =0;                                                     \
          HParsedToken **fields = h_seq_elements(p->ast);               \
           HM_NAME *ret = H_ALLOC( HM_NAME);  /*TODO: can fail*/  \
           __VA_ARGS__                                                 \
                   return h_make(p->arena,(HTokenType)TOKENPASTE2(TT_,HM_NAME), ret); \
       }                
#define HM_STRUCT_SEQ(...) HM_STRUCT_SEQ_IMPL(HM_NAME, __VA_ARGS__)
