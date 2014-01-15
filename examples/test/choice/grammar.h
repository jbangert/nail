#include <nail/macros.h>

N_DEFPARSER(foo, N_ARRAY(
                    N_UINT(uint8_t,h_ch('a'))
                    ,h_many
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
