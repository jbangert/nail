/* Structure definitions  */
#define N_SCALAR(cast,type,parser) type 
#define N_FIELD(name,inner) inner name;
#define N_CONSTANT(inner)
#define N_ARRAY(inner,combinator) struct { inner *elem; size_t count;  } 
#define NX_LENGTHVALUE_HACK(lengthp,elemp) N_ARRAY(elemp, h_length_value)
#define N_STRUCT(inner) struct { inner }
#define N_OPTIONAL(inner) inner *
#define N_DEFPARSER(name,inner) typedef struct name {inner;} name;
#define N_PARSER(name) name 
#define N_REF(name) struct name *
#define N_CHOICE(inner) struct {HTokenType N_type; union{ inner } ; }
#define N_OPTION(name,inner) inner name;
