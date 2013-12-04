#include <stddef.h>
#undef H_STRUCT_SEQ
#undef H__TO 
#undef H_TO_UINT
#undef H_TO_SINT 
#undef H_TO_P   
#undef H_TO_SEQ  

#undef GRAMMAR_BEGIN
#undef GRAMMAR_END
#define GRAMMAR_BEGIN(x)
#define GRAMMAR_END(x)
#ifndef TOKENPASTE2
#define TOKENPASTE(x, y) x ## y  //http://stackoverflow.com/questions/1597007
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#endif 

#ifdef H_MACROS_PARSER
/* Parser -> next include will be actions */
#undef H_MACROS_PARSER 
#define H_MACROS_ACTION
/* Hammer parser */
#define H_TO_UINT H__TO
#define H_TO_SINT H__TO
#define H_TO_P    H__TO
#define H_TO_SEQ  H__TO
#define H__TO(type,field,parser)  parser,
#define H_STRUCT_SEQ(...) HParser *H_NAME = h_action(h_sequence( __VA_ARGS__ NULL),TOKENPASTE2(act_,  H_NAME),NULL);
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
        const struct name * parse_## name(const uint8_t* input, size_t length){ \
                HParseResult * ret = h_parse(init_ ## name (), input,length); \
                if(ret && ret->ast)                                     \
                        return (const struct name *)(ret->ast->user);         \
                else                                                    \
                        return NULL;                                    \
        }

#elif defined(H_MACROS_ACTION)
/*action*/

#undef H_MACROS_ACTION


#define H_TO_UINT(type,field,parser) H__TO(H_CAST_UINT,type,field,parser)
#define H_TO_SINT(type,field,parser) H__TO(H_CAST_SINT,type,field,parser)
#define H_TO_P(type,field,parser)    H__TO(H_CAST_P   ,type,field,parser)
#define H_TO_SEQ(type,field,parser)  H__TO(H_CAST_SEQ ,type,field,parser)

#define H__TO(CAST,type,field,parser)  ret->field = CAST(fields[i]); i++;
#define H_STRUCT_SEQ_IMPL(name,...)                                     \
        HParsedToken * TOKENPASTE2(act_,H_NAME) (const HParseResult *p, void* user_data) \
        {                                                               \
          int i =0;                                                     \
          HParsedToken **fields = h_seq_elements(p->ast);               \
          struct H_NAME *ret = H_ALLOC(struct H_NAME);  /*TODO: can fail*/  \
           __VA_ARGS__                                                 \
                   return h_make(p->arena,(HTokenType)TOKENPASTE2(TT_macro_,H_NAME), ret); \
       }                
#define H_STRUCT_SEQ(...) H_STRUCT_SEQ_IMPL(H_NAME, __VA_ARGS__)
#elif defined(H_MACROS_ENUM)
#undef H_MACROS_ENUM
#define H_MACROS_ERROR1
#define MACROS_END_CURLY
/* Enum - the second include*/

#define H_TO_UINT 
#define H_TO_SINT 
#define H_TO_P    
#define H_TO_SEQ

enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER,
#define H_STRUCT_SEQ(...) TOKENPASTE2(TT_macro_,H_NAME),

#elif  defined(H_MACROS_ERROR1)
#error "macros.h included more than twice without defining H_MACROS_*"
#else
/* Structure definitions  */
#define H_MACROS_ENUM

#define H_STRUCT_SEQ(...) struct H_NAME { __VA_ARGS__ };
#define H__TO(type,field, parser)  type field;  

#define H_TO_UINT H__TO
#define H_TO_SINT H__TO
#define H_TO_P    H__TO
#define H_TO_SEQ  H__TO


#endif
