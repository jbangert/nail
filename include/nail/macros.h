#include <stddef.h>
/*This header file should be included multiple times. 

The User API is to write a header file for the grammar, e.g. grammar.h and structure it as follows
#include <hammer/macros.h>
N_PARSER(N_STRUCT....)
#include <hammer/macros_end.h>
#ifndef N_INCLUDE_DONE
#include "grammar.h" //include  the file itself here
#endif

Whereever the AST is supposed to be accessed, include grammar.h as is.
To produce the functions that create the  AST (in a .c file)
#define N_MACRO_IMPLEMENT
#include "grammar.h"
*/

#include "macros/int-reset.h"

#ifndef N_MACROS_FIRST
#define N_MACROS_FIRST
#include "macros/int-first.h"
#endif


#ifdef N_MACRO_IMPLEMENT
#undef N_INCLUDE_DONE
#undef N_MACRO_IMPLEMENT
#define N_MACROS_ACTION
#endif

#ifdef N_MACROS_PARSER
#undef N_MACROS_PARSER 
#include "macros/int-parser.h"
#define N_MACROS_PRINT
#elif defined(N_MACROS_ACTION)
#undef N_MACROS_ACTION
#include "macros/int-action.h"
/*Run again to emit the actions*/
#define N_MACROS_PARSER
#elif defined(N_MACROS_PRINT)
#undef N_MACROS_PRINT
#include "macros/int-print.h"
#define N_INCLUDE_DONE
#elif defined(N_MACROS_ENUM)
#undef N_MACROS_ENUM
#define N_MACROS_ERROR1
#define N_INCLUDE_DONE
#include "macros/int-enum.h"
#elif  defined(N_MACROS_ERROR1)
#error "macros.h included more than twice without defining N_MACROS_*"
#else
#include "macros/int-struct.h"
/* run again to get the enum */
#define N_MACROS_ENUM

#endif
