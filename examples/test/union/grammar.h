#include <nail/macros.h>

N_DEFPARSER(foo, N_ARRAY(h_many,N_UNION(
                    N_UINT(uint8_t,h_ch('a')),
                    N_UINT(uint8_t,h_ch('b'))
                                 )))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
