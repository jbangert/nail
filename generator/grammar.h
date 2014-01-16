#include <nail/macros.h>
#define al_num h_choice(h_ch_range('a','z'), h_ch_range('A','Z'), h_ch_range('0','9'), h_ch('_'), NULL)
#define space_token(x)  N_CONSTANT(h_whitespace(h_token(x,strlen(x))))  
#define whitespace      N_CONSTANT(h_whitespace(h_token("",0)))
#define comma space_token(",") whitespace
               
#define identifier NX_STRING(al_num, h_many1)

                                                
N_DEFPARSER(parser_parameters,
            N_STRUCT(space_token("(")
                     whitespace
                     N_FIELD(parameters,N_SEPBY(N_REF(parser_invocation),h_ch(',')))
                     whitespace
                     space_token(")")
                                     ))
N_DEFPARSER(parser_invocation,  
            N_STRUCT(whitespace
                     N_FIELD(name,identifier)
                     whitespace
                     N_FIELD(parameters, N_OPTIONAL(N_PARSER(parser_parameters)))
                     whitespace
                    ))
N_DEFPARSER(scalar_rule,
            N_STRUCT(space_token("N_SCALAR")
                     space_token("(")
                     N_FIELD(cast,identifier)
                     comma
                     N_FIELD(type,identifier)
                     comma
                     N_FIELD(parser,N_PARSER(parser_invocation))
                     space_token(")")
                    ))

N_DEFPARSER(embed_rule,
            N_STRUCT(space_token("N_PARSER")
                     space_token("(")
                     N_FIELD(name,identifier)
                     space_token(")")))
N_DEFPARSER(ref_rule,
            N_STRUCT(space_token("N_REF")
                     space_token("(")
                     N_FIELD(name,identifier)
                     space_token(")")))

N_DEFPARSER(parserrule,
            N_CHOICE(N_OPTION(P_STRUCT, N_REF(struct_rule))
                     N_OPTION(P_ARRAY, N_REF(array_rule))
                     N_OPTION(P_REF, N_REF(ref_rule))
                     N_OPTION(P_EMBED, N_REF(embed_rule))
                     N_OPTION(P_SCALAR, N_REF(scalar_rule))
                     N_OPTION(P_OPTIONAL, N_REF(optional_rule))
                     N_OPTION(P_CHOICE,N_REF(choice_rule))
                     N_OPTION(P_NX_LENGTH,N_REF(nx_length_rule))))


N_DEFPARSER(optional_rule,
            N_STRUCT(space_token("N_OPTIONAL")
                     space_token("(")
                     N_FIELD(inner,N_PARSER(parserrule))
                     space_token(")")))
N_DEFPARSER(choice_option,
            N_STRUCT(space_token("N_OPTION")
                     space_token("(")
                     N_FIELD(tag,identifier)
                     comma
                     N_FIELD(inner,N_PARSER(parserrule))
                     space_token(")")))
N_DEFPARSER(choice_rule,
            N_STRUCT(space_token("N_CHOICE")
                     space_token("(")
                     N_FIELD(options,N_ARRAY(N_PARSER(choice_option),h_many))
                     space_token(")")))
N_DEFPARSER(array_rule,
            N_STRUCT(space_token("N_ARRAY")
                     space_token("(")
                     N_FIELD(contents, N_PARSER(parserrule))
                     comma
                     N_FIELD(parser_combinator, identifier)
                     space_token(")")))
N_DEFPARSER(nx_length_rule,
            N_STRUCT(space_token("NX_LENGTHVALUE_HACK")
                     space_token("(")
                     N_FIELD(lengthparser,N_PARSER(parser_invocation))
                     comma
                     N_FIELD(inner,N_PARSER(parserrule))
                     space_token(")")))

N_DEFPARSER(struct_field,
            N_STRUCT(space_token("N_FIELD")
                     space_token("(")
                     N_FIELD(name,identifier)
                     comma
                     N_FIELD(contents,N_PARSER(parserrule))
                     space_token(")")
                    )
        )
N_DEFPARSER(struct_const,
            N_STRUCT(space_token("N_CONSTANT")
                     space_token("(")
                     N_FIELD(contents,N_PARSER(parser_invocation))
                     space_token(")")))                  
N_DEFPARSER(struct_elem,
            N_CHOICE(
                    N_OPTION(S_FIELD, N_PARSER(struct_field))
                    N_OPTION(S_CONST, N_PARSER(struct_const))))
N_DEFPARSER(struct_rule,
            N_STRUCT(
                    space_token("N_STRUCT")
                    space_token("(")
                    N_FIELD(fields,N_ARRAY(N_PARSER(struct_elem),h_many1))
                    space_token(")")
                    ))

N_DEFPARSER(parser_definition,
            N_STRUCT(space_token("N_DEFPARSER")
                     space_token("(")
                     N_FIELD(name,identifier)
                     comma
                     N_FIELD(rule, N_PARSER(parserrule))
                     space_token(")")))


N_DEFPARSER(grammar, N_STRUCT(
                    N_FIELD(rules,N_ARRAY(N_PARSER(parser_definition),h_many))
                    N_CONSTANT(h_whitespace(h_end_p()))))
#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
