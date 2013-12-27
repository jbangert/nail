#define N_DISCARD(inner) inner,
#define N_FIELD(name,inner) h_name(#name, inner ),
#define N_SCALAR(cast,type,parser) parser
#define N_ARRAY(inner,combinator) combinator(inner)
#define N_STRUCT(inner) h_sequence(inner NULL)
#define N_OPTIONAL(inner) h_optional(inner)
#define N_PARSER(name) hammer_x_ ## name()
#define N_DEFPARSER(name,inner) static HParser *hammer_x_ ## name(){      \
                static HParser *ret=NULL;                               \
                if(!ret){                                               \
                        ret= h_name(#name,inner);                       \
                }                                                       \
                return ret;                                             \
        }                                                               \
        HParser *hammer_ ## name(){                                     \
                static HParser *ret=NULL;                               \
                if(!ret){                                               \
                        ret= h_action(hammer_x_##name(),act_## name,NULL); \
                }                                                       \
                return ret;                                             \
        }                                                               \
        const  name * parse_## name(const uint8_t* input, size_t length){ \
                HParseResult * ret = h_parse_error(hammer_ ## name (), input,length,stderr); \
                if(ret && ret->ast)                                     \
                        return (const  name *)(ret->ast->user);         \
                else                                                    \
                        return NULL;                                    \
        }



