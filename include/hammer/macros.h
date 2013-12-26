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
#undef HM_F_OBJECT
#undef HM_UINT
#undef HM_SINT
#undef HM_OBJECT


#define HM_UINT_TYPE(x) x
#define HM_SINT_TYPE(x) x
#define HM_OBJECT_TYPE(x) x*
#define HM_F_OBJECT(type,field)   HM_F(HM_OBJECT,type,field,type)
#define HM_F_OBJECT_OPT(type,field) HM_OPTIONAL(HM_OBJECT,type, field,type,NULL)

#define HM_UINT_CAST(type,val) H_CAST_UINT(val)
#define HM_SINT_CAST(type,val) H_CAST_SINT(val)
#define HM_OBJECT_CAST(type,val) H_CAST(type,val)



#undef HM_ARRAY  
#undef HM_OPTIONAL
#undef GRAMMAR_BEGIN
#undef GRAMMAR_END
#undef HM_RULE

#define GRAMMAR_BEGIN(x)
#define GRAMMAR_END(x)
#define HM_RULE(name,def)

#ifndef HM_MACROS_FIRST
#define HM_MACROS_FIRST
#include "macros/int-first.h"
#endif



#ifdef HM_MACRO_IMPLEMENT
#define HM_MACROS_ACTION
#undef HM_MACRO_IMPLEMENT

#endif

#ifdef HM_MACROS_PARSER
#undef HM_MACROS_PARSER 
#include "macros/int-parser.h"
#define HM_MACROS_PRINT
#define HM_MACRO_INCLUDE_LOOP
#elif defined(HM_MACROS_ACTION)
#undef HM_MACROS_ACTION
#include "macros/int-action.h"
/*Run again to emit the actions*/
#define HM_MACROS_PARSER
#define HM_MACRO_INCLUDE_LOOP

#elif defined(HM_MACROS_PRINT)
#undef HM_MACROS_PRINT
#include "macros/int-print.h"
#elif defined(HM_MACROS_ENUM)
#undef HM_MACROS_ENUM
#define HM_MACROS_ERROR1
#include "macros/int-enum.h"
#elif  defined(HM_MACROS_ERROR1)
#error "macros.h included more than twice without defining HM_MACROS_*"
#else
#include "macros/int-struct.h"
/* run again to get the enum */
#define HM_MACROS_ENUM
#define HM_MACRO_INCLUDE_LOOP
#endif
