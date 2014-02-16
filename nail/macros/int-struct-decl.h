#define N_SCALAR(cast,type,parser) 
#define N_FIELD(name,inner)
#define N_CONSTANT(inner)
#define N_ARRAY(combinator,inner)
#define NX_LENGTHVALUE_HACK(lengthp,elemp)
#define N_STRUCT(inner) 
#define N_OPTIONAL(inner)
#define N_DEFPARSER(name,inner) struct name; typedef struct name name; HParser *hammer_##name(); void print_ ## name(const struct name *val, FILE *out,int indent); int gen_##name(HBitWriter *out,struct name *val); extern  struct  name * parse_## name(const uint8_t* input, size_t length);
#define N_PARSER(name)
#define N_REF(name) *
#define N_CHOICE(inner) 
#define N_OPTION(name,inner) 
