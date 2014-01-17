#include <nail/macros.h>
#define digit  h_ch_range('0','9')
#define al_num h_choice(h_ch_range('a','z'), h_ch_range('A','Z'),digit, h_ch('_'), NULL)
#define number N_ARRAY(N_UINT(uint8_t,digit),h_many1)
#define tok(x)  N_CONSTANT(h_whitespace(h_token(x,strlen(x))))  
#define whitespace      N_CONSTANT(h_whitespace(h_token("",0)))
#define comma tok(",") whitespace
               
#define identifier NX_STRING(al_num, h_many1)
//handle escaped characters here.

N_DEFPARSER(constparser_invocation,
    N_CHOICE( N_OPTION(CHAR,N_STRUCT(tok("h_ch") tok("(") tok("'") N_FIELD(charcode,NX_STRING(h_not_in("'",1),h_many)) N_CONSTANT(h_ch('\''))  tok(")") ))
                ))
N_DEFPARSER(parser_invocation,  
    N_CHOICE(
        N_OPTION(BITS,N_STRUCT(tok("h_bits") tok("(")  N_FIELD(length,number) comma  N_FIELD(sign,number) tok(")"))) //t/f  
        N_OPTION(UINT,N_STRUCT(tok("h_uint")N_FIELD(width,number) tok("(") tok(")")))
        N_OPTION(INT_RANGE,N_STRUCT(
                tok("h_int_range") tok("(") 
                N_FIELD(inner,N_REF(parser_invocation)) comma N_FIELD(lower,number) comma N_FIELD(upper,number) tok(")")))
        ))

N_DEFPARSER(scalar_rule,
    N_STRUCT(tok("N_SCALAR")
        tok("(")
        N_FIELD(cast,identifier)
        comma
        N_FIELD(type,identifier)
        comma
        N_FIELD(parser,N_PARSER(parser_invocation))
        tok(")")
        ))

N_DEFPARSER(embed_rule,
    N_STRUCT(tok("N_PARSER") tok("(")  N_FIELD(name,identifier) tok(")")))
N_DEFPARSER(ref_rule,
    N_STRUCT(tok("N_REF") tok("(") N_FIELD(name,identifier) tok(")")))

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
    N_STRUCT(tok("N_OPTIONAL") tok("(") N_FIELD(inner,N_PARSER(parserrule)) tok(")")))
N_DEFPARSER(choice_option,
    N_STRUCT(tok("N_OPTION")
        tok("(")
        N_FIELD(tag,identifier)
        comma
        N_FIELD(inner,N_PARSER(parserrule))
        tok(")")))
N_DEFPARSER(choice_rule,
    N_STRUCT(tok("N_CHOICE")
        tok("(")
        N_FIELD(options,N_ARRAY(N_PARSER(choice_option),h_many))
        tok(")")))
N_DEFPARSER(array_rule,
    N_STRUCT(tok("N_ARRAY")
        tok("(")
        N_FIELD(contents, N_PARSER(parserrule))
        comma
        N_FIELD(parser_combinator, identifier)
        tok(")")))
N_DEFPARSER(nx_length_rule,
    N_STRUCT(tok("NX_LENGTHVALUE_HACK")
        tok("(")
        N_FIELD(lengthparser,N_PARSER(parser_invocation))
        comma
        N_FIELD(inner,N_PARSER(parserrule))
        tok(")")))

N_DEFPARSER(struct_field,
    N_STRUCT(tok("N_FIELD")
        tok("(")
        N_FIELD(name,identifier)
        comma
        N_FIELD(contents,N_PARSER(parserrule))
        tok(")")
        )
    )
N_DEFPARSER(struct_const,
    N_STRUCT(tok("N_CONSTANT")
        tok("(")
        N_FIELD(contents,N_PARSER(constparser_invocation))
        tok(")")))                  
N_DEFPARSER(struct_elem,
    N_CHOICE(
        N_OPTION(S_FIELD, N_PARSER(struct_field))
        N_OPTION(S_CONST, N_PARSER(struct_const))))
N_DEFPARSER(struct_rule,
    N_STRUCT(
        tok("N_STRUCT")
        tok("(")
        N_FIELD(fields,N_ARRAY(N_PARSER(struct_elem),h_many1))
        tok(")")
        ))

N_DEFPARSER(parser_definition,
    N_STRUCT(tok("N_DEFPARSER")
        tok("(")
        N_FIELD(name,identifier)
        comma
        N_FIELD(rule, N_PARSER(parserrule))
        tok(")")))


N_DEFPARSER(grammar, N_STRUCT(
        N_FIELD(rules,N_ARRAY(N_PARSER(parser_definition),h_many))
        N_CONSTANT(h_whitespace(h_end_p()))))
#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
