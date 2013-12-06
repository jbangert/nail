#include <hammer/macros.h>
GRAMMAR_BEGIN(foo)
#define HM_NAME test_object
HM_STRUCT_SEQ(HM_ARRAY(HM_UINT,char,i1,h_many1(h_ch_range('a','z')))
              HM_F(HM_SINT,int,i2,h_int16()))

#undef HM_NAME
#define HM_NAME foo
HM_STRUCT_SEQ( HM_F(HM_SINT,int,name1,h_int32()) 
//               HM_F_OBJECT(test_object,object))
               HM_OPTIONAL(HM_OBJECT,test_object,object,test_object,NULL))
#undef HM_NAME
GRAMMAR_END(foo)
#include <hammer/macros_end.h>

#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
