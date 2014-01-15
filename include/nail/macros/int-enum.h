/*Define the TT_ enum*/
#define N_MACROS_END_CURLY
/* Enum - the second include*/
#define N_FIELD(name,inner)  inner
#define N_SCALAR(cast,type,parser) 
#define N_ARRAY(inner,combinator) inner
#define N_STRUCT(inner)  inner
#define N_OPTIONAL(inner) inner
#define N_CHOICE(inner) inner
#define N_OPTION(name,inner) ,name
#define N_DISCARD(inner)
#define N_PARSER(name)
#define N_REF(inner)
#define N_DEFPARSER(name,internal) ,_TT_ ## name internal
#define NX_LENGTHVALUE_HACK(lengthp,elemp) elemp
enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER
