#include <stddef.h>
/*This header file should be included multiple times. Depending on defined macros, it defines the
 * HM_TO* and HM_STRUCT_SEQ, etc macros differently:
 Here are the possible actions
 To be included from within the projects header files:
(nothing): Define structures corresponding to the AST for the grammar.  
 HM_MACROS_ENUM: Define the enum HMacroTokenType 

 to be included once per project, in a C file (define HM_MACRO_IMPLEMENT)
 HM_MACROS_PARSER : define a hammer parser for the grammar
 HM_MACROS_ACTION : define actions to emit the AST with the above parser

The User API is to write a header file for the grammar, e.g. grammar.h and structure it as follows
#include <hammer/macros.h>
GRAMMAR_BEGIN(start_rule)
HM_STRUCT_SEQ (... - grammar goes here ...)
GRAMMAR_END(start_rule)
#include <hammer/macros_end.h>
#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h" //include  the file itself here
#endif

Whereever the AST is supposed to be accessed, include grammar.h as is.
To produce the functions that create the  AST (in a .c file)
#define HM_MACRO_IMPLEMENT
#include "grammar.h"
*/

#undef HM_MACRO_INCLUDE_LOOP /* To signal the consumer of this header file that it should include itself */
#undef HM_STRUCT_SEQ
#undef HM__TO 
#undef HM_F
#undef HM_PTR
#undef HM_F_OBJECT

#define HM_F_OBJECT(type,field)   HM_F(HM_OBJECT,HM_PTR(type),field,type)
#define HM_UINT(type,val) H_CAST_UINT(val)
#define HM_SINT(type,val) H_CAST_SINT(val)
#define HM_OBJECT(type,val) H_CAST(type,val)
#define HM_PTR(type) type *

#undef HM_ARRAY  

#undef GRAMMAR_BEGIN
#undef GRAMMAR_END
#undef HM_RULE
#define GRAMMAR_BEGIN(x)
#define GRAMMAR_END(x)
#define HM_RULE(name,def)
#ifndef TOKENPASTE2
#define TOKENPASTE(x, y) x ## y  //http://stackoverflow.com/questions/1597007
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#endif 

#ifdef HM_MACRO_IMPLEMENT
#define HM_MACROS_ACTION
#undef HM_MACRO_IMPLEMENT

#endif

#ifdef HM_MACROS_PARSER

#undef HM_MACROS_PARSER 

/* Hammer parser */



#define HM_F(cast,type,field,parser)  parser,
#define HM_ARRAY(aggr,cast,type,field,parser) aggr(parser),
#define HM_STRUCT_SEQ(...) HParser *HM_NAME = h_action(h_sequence( __VA_ARGS__ NULL),TOKENPASTE2(act_,  HM_NAME),NULL);
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


#elif defined(HM_MACROS_ACTION)
/*action*/

#undef HM_MACROS_ACTION
#undef HM_PTR
#define HM_PTR(type) type 
#define HM_F(cast,type,field,parser) ret->field = cast(type,fields[i]); i++;
#define HM_ARRAY(aggr,cast,type,field,parser) ret->field.count = h_seq_len(fields[i]); \
        do{int j; HParsedToken **seq = h_seq_elements(fields[i]); \
        ret->field.elem = (type *)h_arena_malloc(p->arena, sizeof(type) * ret->field.count); /*  WARNING, can fail*/ \
        for(j=0;j<ret->field.count;j++){ret->field.elem[j] = cast(type,seq[j]); } \
        }while(0); i++;
//#define HM_F_ARRAY(type,field,parser)  ret->field = /* We want an array of type */
// HM__TO(H_CAST_SEQ ,type,field,parser)


#define HM_STRUCT_SEQ_IMPL(name,...)                                     \
        HParsedToken * TOKENPASTE2(act_,HM_NAME) (const HParseResult *p, void* user_data) \
        {                                                               \
          int i =0;                                                     \
          HParsedToken **fields = h_seq_elements(p->ast);               \
          struct HM_NAME *ret = H_ALLOC(struct HM_NAME);  /*TODO: can fail*/  \
           __VA_ARGS__                                                 \
                   return h_make(p->arena,(HTokenType)TOKENPASTE2(TT_,HM_NAME), ret); \
       }                
#define HM_STRUCT_SEQ(...) HM_STRUCT_SEQ_IMPL(HM_NAME, __VA_ARGS__)
#undef HM_RULE
#define HM_RULE(name,def) H_RULE(name,def)
/*Run again to emit the actions*/
#define HM_MACROS_PARSER
#define HM_MACRO_INCLUDE_LOOP


#elif defined(HM_MACROS_ENUM)
#undef HM_MACROS_ENUM
#define HM_MACROS_ERROR1
#define MACROS_END_CURLY
/* Enum - the second include*/

#define HM_F
#define HM_ARRAY

enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER,
#define HM_STRUCT_SEQ(...) TOKENPASTE2(TT_,HM_NAME),

#elif  defined(HM_MACROS_ERROR1)
#error "macros.h included more than twice without defining HM_MACROS_*"
#else
/* Structure definitions  */
#define HM_STRUCT_SEQ(...) typedef struct HM_NAME { __VA_ARGS__ } HM_NAME;
#define HM_F(cast,type,field, parser)  type field;  
#undef GRAMMAR_END
#define GRAMMAR_END(name) extern const struct name *parse_ ## name(const uint8_t *input, size_t length); \
        extern const HParser * init_ ## name(); 



#define HM_ARRAY(aggr,cast,type,field,parser)  struct { type *elem; size_t count; } field;

/* run again to get the enum */
#define HM_MACROS_ENUM
#define HM_MACRO_INCLUDE_LOOP
#endif
