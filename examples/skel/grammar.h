#include <nail/macros.h>

N_DEFPARSER(foo, N_STRUCT(
                    N_FIELD(elements,
                            N_ARRAY(h_many,N_PARSER(inner_struct)))
                    N_CONSTANT(h_end_p())
                    ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
