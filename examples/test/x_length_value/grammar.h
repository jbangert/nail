#include <nail/macros.h>

N_DEFPARSER(foo, N_STRUCT(
                    N_FIELD(elements,
                            NX_LENGTHVALUE_HACK(h_uint8(),N_UINT(unsigned char,h_uint8())))
                    N_CONSTANT(h_end_p())
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
