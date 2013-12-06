#include <hammer/macros.h>
GRAMMAR_BEGIN(foo)
#define HM_NAME test_object
HM_STRUCT_SEQ(HM_F(HM_SINT,int,i1,h_int16())
              HM_F(HM_SINT,int,i2,h_int16()))
#undef HM_NAME
#define HM_NAME foo
HM_STRUCT_SEQ( HM_F(HM_SINT,int,name1,h_int32()) 
               HM_F_OBJECT(test_object,object))
//               HM_F(HM_OBJECT,HM_PTR(test_object),object,test_object))
#undef HM_NAME
GRAMMAR_END(foo)
#include <hammer/macros_end.h>

#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
