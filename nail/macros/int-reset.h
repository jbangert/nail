#undef N_STRUCT
#undef N_ARRAY
#undef N_FIELD
#undef N_SCALAR
#undef N_DEFPARSER
#undef N_OPTIONAL
#undef N_CONSTANT
#undef N_PARSER
#undef N_CHOICE
#undef N_OPTION
#undef N_REF
#undef N_SEPBY
#define N_SEPBY(separator,inner) N_ARRAY(_COMPILE_ERROR,inner)
#undef N_WRAP


#undef N_UNION
#undef NX_STRING
#define NX_STRING(char,combinator) N_ARRAY(combinator,N_UINT(uint8_t,char))
#undef NX_LENGTHVALUE_HACK
#undef NX_HRULE
#define NX_HRULE(name, inner)

#undef h_bits
#undef h_uint8
#undef h_uint16
#undef h_uint32
#undef h_uint64
#undef h_sint8
#undef h_sint16
#undef h_sint32
#undef h_sint64
