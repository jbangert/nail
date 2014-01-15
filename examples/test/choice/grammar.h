#include <nail/macros.h>

N_DEFPARSER(foo, N_ARRAY(N_CHOICE(N_OPTION(CH_A,N_UINT(uint8_t,h_ch('a')))
                                  N_OPTION(CH_B,N_UINT(uint8_t,h_ch('b')))),h_many))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
