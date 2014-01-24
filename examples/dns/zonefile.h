#include <nail/macros.h>
#define letter  h_choice(h_ch_range('a','z'), h_ch_range('A','Z'), NULL)
#define let_dig h_choice(letter, h_ch_range('0','9'), NULL)

N_DEFPARSER(domain, N_SEPBY(NX_STRING(let_dig,h_many1),'.'))
N_DEFPARSER(rr_cname, N_WRAP(N_CONSTANT(h_token("CNAME:")),
                             N_PARSER(domain),))
N_DEFPARSER(rr_mx, N_WRAP(N_CONSTANT(h_token("MX:")),
                       N_PARSER(domain),))
N_DEFPARSER(rr_ns, N_WRAP(N_CONSTANT(h_token("NS:")),
                       N_PARSER(domain),))
#define ip_octet N_UINT(uint8_t,h_int_range(0,255,h_strint(0)))
N_DEFPARSER(ip,N_STRUCT(N_FIELD(oct1,ip_octet) 
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct2,ip_octet)
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct3,ip_octet)
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct4,ip_octet)))
N_DEFPARSER(rr_a, N_WRAP(N_CONSTANT(h_token("A:")),
                      N_PARSER(ip),))
N_DEFPARSER(zonerecord,
            N_STRUCT(N_FIELD(name,N_PARSER(domain))
                     N_FIELD(value,N_CHOICE(
                                     N_OPTION(CNAME,N_PARSER(rr_cname))
                                     N_OPTION(A,N_PARSER(rr_a))
                                     N_OPTION(NS,N_PARSER(rr_ns))
                                     N_OPTION(MX,N_PARSER(rr_mx))))
                     N_FIELD(ttl,N_UINT(uint32_t,h_strint(0)))
                     N_CONSTANT(h_ch('\n'))))
            
#include <nail/macros_end.h>

#ifndef N_INCLUDE_DONE
#include "zonefile.h"
#endif



