/* Structure definitions  */
#define N_SCALAR(cast,type,parser) type 
#define N_FIELD(name,inner) inner name;
#define N_DISCARD(inner)
#define N_ARRAY(inner,combinator) struct { inner *elem; size_t count;  } 
#define N_STRUCT(inner) struct { inner }
#define N_OPTIONAL(inner) inner *
#define N_PARSER(name,inner) typedef inner name;
