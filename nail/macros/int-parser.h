HParsedToken *N_act_choice_tag(const HParseResult *p, void *user_data) {
        return h_make(p->arena,(HTokenType)user_data,(void *)p->ast); /* Wrap pointer in a thin struct*/
}
#define N_SCALAR(cast,type,parser) parser
#define N_OPTIONAL(inner) h_optional(inner)

#define N_STRUCT(inner) h_sequence(inner NULL)
#define N_CONSTANT(inner) inner,
#define N_FIELD(name,inner) h_name(#name, inner ),

#define N_ARRAY(inner,combinator) combinator(inner)
#undef N_SEPBY
#define N_SEPBY(inner,seperator) h_sepBy(inner,seperator)
#define NX_LENGTHVALUE_HACK(lengthp, elemp) h_length_value(lengthp,elemp)
#undef NX_HRULE
#define NX_HRULE(name,inner) H_RULE(name, inner);
#define N_PARSER(name) hammer_x_## name()
#define N_REF(name) hammer_##name()
#define N_CHOICE(inner) h_choice(inner NULL)
#define N_OPTION(name,inner) h_action(h_name(#name,inner),N_act_choice_tag,(void *) name),
#define N_DEFPARSER(name,inner) static HParser *hammer_x_ ## name(){      \
                static HParser *ret=NULL;                               \
                if(!ret){                                               \
                        ret = h_name(#name,h_indirect());               \
                        h_bind_indirect(ret,h_name(#name,inner));       \
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




