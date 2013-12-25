#include <hammer/macros.h>
#include <hammer/glue.h>
#define false 0
GRAMMAR_BEGIN(dns_message)
HM_RULE(letter, h_choice(h_ch_range('a','z'), h_ch_range('A','Z'),NULL))
HM_RULE(let_dig,   h_choice(letter, h_ch_range('0','9'), NULL))
HM_RULE(ldh_char,   h_choice(let_dig, h_ch('-'), NULL))

#define HM_NAME dns_label_tail
HM_STRUCT_SEQ( HM_ARRAY( HM_UINT,char, other_letters,h_optional(h_many1(ldh_char)))
               HM_F(HM_UINT,char,last_letter,let_dig))
#undef HM_NAME
#define HM_NAME dns_label
HM_STRUCT_SEQ(HM_F(HM_UINT,char,firstletter,letter)
              HM_F_OBJECT_OPT(dns_label_tail,tail))
#undef HM_NAME
#define HM_NAME dns_domain
HM_STRUCT_SEQ(HM_ARRAY(HM_OBJECT,dns_label, labels,h_sepBy1(dns_label,h_ch('.'))))
//TODO: Make this a choice!
#undef HM_NAME



#define HM_NAME domain_label

#undef HM_NAME


#define HM_NAME header
HM_STRUCT_SEQ(HM_F(HM_UINT,uint16_t, id,h_bits(16,false))
             HM_F(HM_UINT,bool, qr,h_bits(1,false))
             HM_F(HM_UINT,char,opcode,h_bits(4,false))
             HM_F(HM_UINT,bool, aa,h_bits(1,false))
             HM_F(HM_UINT,bool, tc,h_bits(1,false))
             HM_F(HM_UINT,bool, rd,h_bits(1,false))
             HM_F(HM_UINT,bool, ra,h_bits(1,false))
             HM_F(HM_UINT,char, Z, h_bits(3,false)) 
             HM_F(HM_UINT,char, rcode, h_bits(4,false))
             HM_F(HM_UINT,size_t, question_count, h_uint16())
             HM_F(HM_UINT,size_t, answer_count, h_uint16())
             HM_F(HM_UINT,size_t, authority_count, h_uint16())
             HM_F(HM_UINT,size_t, additional_count, h_uint16()))

#undef HM_NAME
HM_RULE  (type,     h_int_range(h_uint16(), 1, 16))
HM_RULE  (qtype,    h_choice(type, 
        h_int_range(h_uint16(), 252, 255),
        NULL))
HM_RULE  (class,    h_int_range(h_uint16(), 1, 4))
HM_RULE  (qclass,   h_choice(class,
        h_int_range(h_uint16(), 255, 255),
        NULL))
HM_RULE  (len,      h_int_range(h_uint8(), 1, 255))
HM_RULE (qlabel,    h_length_value(len, h_uint8()))  /* We need an exact length tracing combinator
                                                       * Gahk */


#define HM_NAME question 
HM_STRUCT_SEQ(HM_ARRAY(HM_UINT,char,labels, qlabel)
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
