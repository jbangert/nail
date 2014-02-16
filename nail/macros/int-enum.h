/*Define the TT_ enum*/
#define N_MACROS_END_CURLY
/* Enum - the second include*/
#define N_FIELD(name,...)  __VA_ARGS__
#define N_SCALAR(cast,type,parser) 
#define N_ARRAY(combinator,...) __VA_ARGS__

#define N_STRUCT(...)  __VA_ARGS__
#define N_WRAP(a,b,c) b
#define N_OPTIONAL(...) __VA_ARGS__
#define N_CHOICE(...) __VA_ARGS__
#define N_UNION(first,...) first
#define N_OPTION(name,...) ,name
#define N_CONSTANT(inner)
#define N_PARSER(name)
#define N_REF(inner)
#define N_DEFPARSER(name,...) ,TT_ ## name __VA_ARGS__
#define NX_LENGTHVALUE_HACK(lengthp,elemp) elemp
enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER
