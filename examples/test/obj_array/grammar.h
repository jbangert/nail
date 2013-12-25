#include <hammer/macros.h>
GRAMMAR_BEGIN(obj_array)
#define HM_NAME s1
HM_STRUCT_SEQ(HM_F(HM_UINT,uint8_t,a1,h_uint8())
              HM_F(HM_UINT,uint8_t,a2,h_uint8()))
#undef HM_NAME
#define HM_NAME obj_array
HM_STRUCT_SEQ(HM_ARRAY(HM_OBJECT,s1,entry1,h_many(s1))
              )
#undef HM_NAME
GRAMMAR_END(obj_array)
#include <hammer/macros_end.h>

#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
