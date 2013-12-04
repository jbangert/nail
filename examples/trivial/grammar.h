#include <hammer/macros.h>
GRAMMAR_BEGIN(foo)
#define H_NAME foo
H_STRUCT_SEQ( H_TO_SINT(int,name1,h_int32()) 
              H_TO_SINT(int,name2,h_int32()))
#undef H_NAME
GRAMMAR_END(foo)
#include <hammer/macros_end.h>

#ifdef H_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
