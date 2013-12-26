/* Structure definitions  */
#define HM_OPTIONAL(cast,type,field,parser,default) HM_F(cast,type,field,parser)
#define HM_STRUCT_SEQ(...) typedef struct HM_NAME { __VA_ARGS__ } HM_NAME; extern void TOKENPASTE2(print_,HM_NAME)(const HM_NAME *ptr,FILE *out,int indent);
#define HM_F(cast,type,field, parser)  HM_DO_TYPE(cast,type) field;  
#undef GRAMMAR_END
#define GRAMMAR_END(name) extern const  name *parse_ ## name(const uint8_t *input, size_t length); \
        extern const HParser * init_ ## name(); 
#define HM_ARRAY(cast,type,field,parser)  struct { HM_DO_TYPE(cast,type) *elem; size_t count; } field;
