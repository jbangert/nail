#include <hammer/macros.h>
GRAMMAR_BEGIN(foo)
#define HM_NAME foo
HM_STRUCT_SEQ( HM_TO_SINT(int,name1,h_int32()) 
              HM_TO_SINT(int,name2,h_int32()))
#undef HM_NAME
GRAMMAR_END(foo)
#include <hammer/macros_end.h>

#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
