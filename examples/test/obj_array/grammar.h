#include <nail/macros.h>
N_DEFPARSER(inner_struct, N_STRUCT(
                    N_FIELD(a1,N_SCALAR(UINT,int,h_uint8()))            
                    N_FIELD(a2,N_OPTIONAL(N_SCALAR(UINT,int,h_uint8())))))
/*N_DEFPARSER(obj_array,
            N_STRUCT(N_FIELD(elements,N_ARRAY( N_STRUCT(
                    N_FIELD(a1,N_SCALAR(UINT,int,h_uint8()))            
                    N_FIELD(a2,N_OPTIONAL(N_SCALAR(UINT,int,h_uint8())))) ,h_many))
                    N_CONSTANT(h_end_p()))) */
N_DEFPARSER(obj_array, N_STRUCT(
                    N_FIELD(elements,
                            N_ARRAY(h_many,N_PARSER(inner_struct)))
                    N_CONSTANT(h_end_p())
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
