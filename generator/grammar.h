#include <nail/macros.h>
#define N_CONSTANT N_DISCARD
#define al_num h_choice(h_ch_range('a','z'), h_ch_range('A','Z'), h_ch_range('0','9'), h_ch('_'), NULL)
#define space_token(x)                          \
        N_CONSTANT(h_whitespace(h_token(x,strlen(x))))   



N_DEFPARSER(balanced_paren,
            N_ARRAY(N_CHOICE(N_OPTION(P_CHAR, N_UINT(uint8_t,h_not_in("()",2)))
                             N_OPTION(P_NEST, N_STRUCT(N_CONSTANT(h_ch('('))
                                                       N_FIELD(contents,N_REF(balanced_paren))
                                                       N_CONSTANT(h_ch(')'))))),h_many))
N_DEFPARSER(identifier, N_ARRAY(N_UINT(uint8_t,al_num), h_many1))
N_DEFPARSER(parser_parameters,
            N_STRUCT(space_token("(")
                     N_FIELD(parameters,N_SEPBY(N_REF(parser_invocation),h_ch(',')))
                     space_token(")")
                    ))
N_DEFPARSER(parser_invocation,  
            N_STRUCT(space_token("")
                     N_FIELD(name,N_PARSER(identifier))
                     N_FIELD(parameters, N_OPTIONAL(N_PARSER(parser_parameters)))
                    ))
N_DEFPARSER(scalar_rule,
            N_STRUCT(space_token("N_SCALAR")
                     space_token("(")
                     N_FIELD(cast,N_PARSER(identifier))
                     space_token(",")
                     N_FIELD(type,N_PARSER(identifier))
                     space_token(",")
                     N_FIELD(parser,N_PARSER(parser_invocation))
                     space_token(")")
                    ))

N_DEFPARSER(parserrule,
            N_CHOICE(N_OPTION(P_STRUCT, N_REF(struct_rule))
                     N_OPTION(P_ARRAY, N_REF(array_rule))
                     N_OPTION(P_SCALAR, N_REF(scalar_rule))))
N_DEFPARSER(array_rule,
            N_STRUCT(space_token("N_ARRAY")
                     space_token("(")
                     N_FIELD(contents, N_PARSER(parserrule))
                     space_token(",")
                     N_FIELD(parser_combinator, N_PARSER(identifier))
                     space_token(")")))


N_DEFPARSER(struct_field,
            N_STRUCT(space_token("N_FIELD")
                     space_token("(")
                     N_FIELD(name,N_PARSER(identifier))
                     space_token(",")
                     N_FIELD(contents,N_PARSER(parserrule))
                     space_token(")")
                    )
        )
N_DEFPARSER(struct_const,
            N_STRUCT(space_token("N_CONSTANT")
                     space_token("(")
                     N_FIELD(contents,N_PARSER(parserrule))))                  
N_DEFPARSER(struct_elems,
            N_CHOICE(
                    N_OPTION(S_FIELD, N_PARSER(struct_field))
                    N_OPTION(S_CONST, N_PARSER(struct_const))))
N_DEFPARSER(struct_rule,
            N_STRUCT(
                    space_token("N_STRUCT")
                    space_token("(")
                    N_FIELD(fields,N_ARRAY(N_PARSER(struct_elems),h_many1))
                    space_token(")")
                    ))

N_DEFPARSER(parser_definition,
            N_STRUCT(space_token("N_DEFPARSER")
                     space_token("(")
                     N_FIELD(name,N_PARSER(identifier))
                     space_token(",")
                     N_FIELD(rule, N_PARSER(parserrule))
                     space_token(")")))


N_DEFPARSER(grammar, N_ARRAY(N_PARSER(parser_definition),h_many))

 

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
