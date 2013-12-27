#include <hammer/hammer.h>
#include <hammer/glue.h>
#include <nail/macros.h>

#define false 0
/* Do away with these once we have N_UNION */
H_RULE(letter, h_choice(h_ch_range('a','z'), h_ch_range('A','Z'),NULL))
H_RULE(let_dig,   h_choice(letter, h_ch_range('0','9'), NULL))
H_RULE(ldh_char,   h_choice(let_dig, h_ch('-'), NULL))


N_DEFPARSER(header,
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


H_RULE  (type,     h_int_range(h_uint16(), 1, 16))
H_RULE  (qtype,    h_choice(type, 
        h_int_range(h_uint16(), 252, 255),
        NULL))
H_RULE  (class,    h_int_range(h_uint16(), 1, 4))
H_RULE  (qclass,   h_choice(class,
        h_int_range(h_uint16(), 255, 255),
        NULL))
H_RULE  (len,      h_int_range(h_uint8(), 1, 255))
H_RULE (qlabel,    h_length_value(len, h_uint8()))  /* We need an exact length tracing combinator
                                                       * Gahk */


N_STRUCT(N_FIELD(labels,
#define HM_NAME question 
HM_STRUCT_SEQ(HM_ARRAY(HM_UINT,char,labels, h_many(qlabel))
HM_F(HM_UINT,uint8_t,nullbyte, h_ch('\x00'))
HM_F(HM_UINT,int, qtype,qtype)
HM_F(HM_UINT,int,qclass,qclass))

#undef HM_NAME
#define HM_NAME dns_message
HM_STRUCT_SEQ(HM_F_OBJECT(header, hdr)
HM_ARRAY(HM_OBJECT,question,questions,h_many(question))
HM_F(HM_UINT,uint8_t,endofmessage,h_end_p()))
#undef HM_NAME
GRAMMAR_END(dns_message)


#include <hammer/macros_end.h>

#ifdef HM_MACRO_INCLUDE_LOOP
#include "grammar.h"
#endif
