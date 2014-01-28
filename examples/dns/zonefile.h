#define letter  h_choice(h_ch_range('a','z'), h_ch_range('A','Z'), NULL)
#define let_dig h_choice(letter, h_ch_range('0','9'), NULL)
#define tok(x)  N_CONSTANT(h_whitespace(h_token(x,strlen(x))))  

N_DEFPARSER(label, NX_STRING(let_dig,h_many1))
/*
N_DEFPARSER(auth, N_STRUCT(
                    tok("")
                    N_FIELD(label,N_PARSER(label))
                    tok("auth")
                    tok("{")
                    N_FIELD(many,N_ARRAY(N_PARSER(auth_records)),h_many1
                    tok("}")
                    ))
*/

N_DEFPARSER(domain, N_SEPBY(N_PARSER(label),'.'))
N_DEFPARSER(rr_ns, N_STRUCT(N_CONSTANT(h_token("NS:"))
                            N_FIELD(domain,N_PARSER(domain))))
N_DEFPARSER(rr_mx, N_STRUCT(N_CONSTANT(h_token("MX:"))
                            N_FIELD(domain,N_PARSER(domain))))
N_DEFPARSER(rr_cname, N_STRUCT(N_CONSTANT(h_token("CNAME:"))
                            N_FIELD(domain,N_PARSER(domain))))
#define ip_octet N_UINT(uint8_t,h_int_range(h_strint(0),0,255))
N_DEFPARSER(ip,N_STRUCT(N_FIELD(oct1,ip_octet) 
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct2,ip_octet)
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct3,ip_octet)
                        N_CONSTANT(h_ch('.'))
                        N_FIELD(oct4,ip_octet)))
N_DEFPARSER(rr_a, N_STRUCT(N_CONSTANT(h_token("A:"))
                           N_FIELD(ip,N_PARSER(ip))))
N_DEFPARSER(zonerecord,
            N_STRUCT(N_FIELD(name,N_PARSER(domain))
                     N_FIELD(value,N_CHOICE(
                                     N_OPTION(CNAME,N_PARSER(rr_cname))
                                     N_OPTION(A,N_PARSER(rr_a))
                                     N_OPTION(NS,N_PARSER(rr_ns))
                                     N_OPTION(MX,N_PARSER(rr_mx))))
                     N_FIELD(ttl,N_UINT(uint32_t,h_strint(0)))
                     N_CONSTANT(h_ch('\n'))))
            



