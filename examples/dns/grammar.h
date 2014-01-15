#include <nail/macros.h>

#define false 0
/* Do away with these once we have N_UNION */

N_DEFPARSER(dns_header,
N_STRUCT(N_FIELD(id,     N_UINT(uint16_t,h_bits(16,false)))
         N_FIELD(qr,     N_UINT(bool,h_bits(1,false)))
         N_FIELD(opcode, N_UINT(char,h_bits(4,false)))
         N_FIELD(aa,     N_UINT(bool,h_bits(1,false)))
         N_FIELD(tc,     N_UINT(bool,h_bits(1,false)))
         N_FIELD(rd,     N_UINT(bool,h_bits(1,false)))
         N_FIELD(ra,     N_UINT(bool,h_bits(1,false)))
         N_FIELD(Z,      N_UINT(char,h_bits(3,false)))
         N_FIELD(rcode,  N_UINT(char,h_bits(4,false)))
         N_FIELD(question_count,  N_UINT(size_t,h_uint16()))
         N_FIELD(answer_count,    N_UINT(size_t,h_uint16()))
         N_FIELD(authority_count, N_UINT(size_t,h_uint16()))
         N_FIELD(additional_count,N_UINT(size_t,h_uint16()))
        ))
N_DEFPARSER(dns_question,
            N_STRUCT(N_FIELD(labels,N_ARRAY(
                                     NX_LENGTHVALUE_HACK(h_int_range(h_uint8(),1,255),N_UINT(char,h_uint8())),h_many1))
                     N_CONSTANT(h_ch(0))
                     N_FIELD(qtype,N_UINT(int, h_choice( h_int_range(h_uint16(), 1, 16), h_int_range(h_uint16(), 252, 255), NULL)))
                     N_FIELD(qclass,N_UINT(int, h_choice(h_int_range(h_uint16(), 1, 4),h_int_range(h_uint16(), 255, 255), NULL)))))

N_DEFPARSER(dns_message,
                    N_STRUCT( N_FIELD(header,N_PARSER(dns_header))
                              N_FIELD(questions,N_ARRAY(N_PARSER(dns_question),h_many))
                            ))

#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "grammar.h"
#endif
