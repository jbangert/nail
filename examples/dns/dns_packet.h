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
N_DEFPARSER(dns_labels,N_WRAP(
                    ,N_ARRAY(h_many1,NX_LENGTHVALUE_HACK(h_int_range(h_uint8(),1,255),N_UINT(char,h_uint8()))),
                    N_CONSTANT(h_ch('\x00'))))
N_DEFPARSER(dns_question,
            N_STRUCT(N_FIELD(labels,N_PARSER(dns_labels))
                     N_FIELD(qtype,N_UINT(int, h_choice( h_int_range(h_uint16(), 1, 16), h_int_range(h_uint16(), 252, 255), NULL)))
                     N_FIELD(qclass,N_UINT(int, h_choice(h_int_range(h_uint16(), 1, 4),h_int_range(h_uint16(), 255, 255), NULL)))))

N_DEFPARSER(dns_response,
            N_STRUCT(N_FIELD(labels,N_PARSER(dns_labels))
                     N_FIELD(rtype,N_UINT(uint16_t,h_int_range(h_uint16(),1,16)))
                     N_FIELD(class,N_UINT(uint16_t,h_int_range(h_uint16(),1,255)))
                     N_FIELD(ttl,N_UINT(uint32_t,h_uint32()))
                     N_FIELD(rdata,NX_LENGTHVALUE_HACK(h_uint16(),N_UINT(char,h_uint8())))))
N_DEFPARSER(dns_message,
            N_STRUCT( N_FIELD(header,N_PARSER(dns_header))
                      N_FIELD(questions,N_ARRAY(h_many,N_PARSER(dns_question)))
                      N_FIELD(rr,N_ARRAY(h_many,N_PARSER(dns_response)))
                      N_CONSTANT(h_end_p())
                    ))
  
