/*Define the TT_ enum*/
#define MACROS_END_CURLY
/* Enum - the second include*/

#define HM_F
#define HM_ARRAY

enum HMacroTokenType_  {
        TT_Macro_unused = TT_USER,
#define HM_STRUCT_SEQ(...) TOKENPASTE2(TT_,HM_NAME),
