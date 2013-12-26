/* Hammer parser */

#define XQUOTE(x) #x
#define QUOTE(x) XQUOTE(x)

#define HM_F(cast,type,field,parser)  h_name(# field, parser),
#undef HM_OPTIONAL
#define HM_OPTIONAL(cast,type,field,parser,default) h_name( #field, h_optional(parser)),
#define HM_ARRAY HM_F
#define HM_STRUCT_SEQ(...) HParser *HM_NAME = h_action(h_name(QUOTE(HM_NAME),h_sequence( __VA_ARGS__ NULL)),TOKENPASTE2(act_,  HM_NAME),NULL);
#undef HM_RULE
#define HM_RULE(name,def) H_RULE(name,h_name(#name,def));
#undef GRAMMAR_BEGIN
#undef GRAMMAR_END
#define GRAMMAR_BEGIN(name)                     \
        const HParser * init_ ## name ()        \
        {                                       \
           static const HParser *ret = NULL;    \
           if(ret)                              \
                   return ret;                  
#define GRAMMAR_END(name)                       \
        ret = name;                             \
        return ret;                             \
        }                                       \
        const  name * parse_## name(const uint8_t* input, size_t length){ \
                HParseResult * ret = h_parse_error(init_ ## name (), input,length,stderr); \
                if(ret && ret->ast)                                     \
                        return (const  name *)(ret->ast->user);         \
                else                                                    \
                        return NULL;                                    \
        }
