#include <nail/macros.h>

N_DEFPARSER(foo, N_STRUCT(
                    N_FIELD(elements,
                            N_ARRAY(N_PARSER(inner_struct),h_many))
                    N_DISCARD(h_end_p())
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
