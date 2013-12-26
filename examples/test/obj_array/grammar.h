#include <nail/macros.h>
N_PARSER(obj_array,
         N_ARRAY(N_STRUCT(N_FIELD(a1,N_SCALAR(UINT,int,h_uint8()))
                          N_FIELD(a2,N_SCALAR(UINT,int,h_uint8()))),h_many))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
