/*Define the TT_ enum*/
#define N_MACROS_END_CURLY
/* Enum - the second include*/

#define N_FIELD(name,inner) 
#define N_SCALAR(cast,type,parser) 
#define N_ARRAY(inner,combinator) 
#define N_STRUCT(inner) 
#define N_OPTIONAL(inner)
enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER,
#define N_DEFPARSER(name,intermal) TT_ ## name, 
#define NX_LENGTHVALUE_HACK(lengthp,elemp)
