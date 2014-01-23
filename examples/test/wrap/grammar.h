#include <nail/macros.h>

N_DEFPARSER(foo, N_WRAP(
                    N_CONSTANT(h_ch('(')),
                    N_STRUCT(N_FIELD(test,N_UINT(uint8_t,h_uint8()))),
                    N_CONSTANT(h_ch(')'))
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
