#include <stdint.h>
#include <string.h>
#include <assert.h>
enum N_types {_NAIL_NULL,BIG,LITTLE,ENDIAN,CONSTANTDEF,PARSER,PR,PAREN,NAME,REF,NUNION,OPTIONAL,APPLY,LENGTH,FIXEDARRAY,ARRAY,SELECTP,CHOICE,WRAP,STRUCTURE,INTEGER,DDEPENDENCY,DSTREAM,PSTREAM,PDEPENDENCY,SEPBY,SEPBYONE,MANY,MANYONE,FIELD,TRANSFORM,DEPENDENCY,CONSTANT,NEGATE,SET,SINGLE,INTVALUE,RANGE,CUNION,CSTRUCT,CREF,CINT,CREPEAT,CARRAY,VALUES,STRING,SIGN,UNSIGN,DIRECT,ESCAPE,NUMBER,HEX,ASCII};
typedef struct number number;
typedef struct varidentifier varidentifier;
typedef struct constidentifier constidentifier;
typedef struct streamidentifier streamidentifier;
typedef struct dependencyidentifier dependencyidentifier;
typedef struct intconstant intconstant;
typedef struct intp intp;
typedef struct constint constint;
typedef struct arrayvalue arrayvalue;
typedef struct constarray constarray;
typedef struct constfields constfields;
typedef struct constparser constparser;
typedef struct constraintelem constraintelem;
typedef struct intconstraint intconstraint;
typedef struct constrainedint constrainedint;
typedef struct transform transform;
typedef struct structparser structparser;
typedef struct wrapparser wrapparser;
typedef struct choiceparser choiceparser;
typedef struct selectparser selectparser;
typedef struct arrayparser arrayparser;
typedef struct parameter parameter;
typedef struct parameterlist parameterlist;
typedef struct parameterdefinition parameterdefinition;
typedef struct parameterdefinitionlist parameterdefinitionlist;
typedef struct parserinvocation parserinvocation;
typedef struct parserinner parserinner;
typedef struct parser parser;
typedef struct definition definition;
typedef struct grammar grammar;
struct number {
    uint8_t*elem;
    size_t count;
};
struct varidentifier {
    uint8_t*elem;
    size_t count;
};
struct constidentifier {
    uint8_t*elem;
    size_t count;
};
struct streamidentifier {
    uint8_t*elem;
    size_t count;
};
struct dependencyidentifier {
    uint8_t*elem;
    size_t count;
};
struct intconstant {
    enum N_types N_type;
    union {
        struct {
            enum N_types N_type;
            union {
                uint8_t escape;
                uint8_t direct;
            };
        } ascii;
        struct {
            uint8_t*elem;
            size_t count;
        } hex;
        number number;
    };
};
struct intp {
    enum N_types N_type;
    union {
        struct {
            uint8_t*elem;
            size_t count;
        } unsign;
        struct {
            uint8_t*elem;
            size_t count;
        } sign;
    };
};
struct constint {
    intp parser;
    intconstant value;
}
;
struct arrayvalue {
    enum N_types N_type;
    union {
        struct {
            uint8_t*elem;
            size_t count;
        } string;
        struct {
            intconstant*elem;
            size_t count;
        } values;
    };
};
struct constarray {
    intp parser;
    arrayvalue value;
}
;
struct constfields {
    constparser*elem;
    size_t count;
};
struct constparser {
    enum N_types N_type;
    union {
        constarray carray;
        constparser*  crepeat;
        constint cint;
        constidentifier cref;
        constfields cstruct;
        struct {
            constparser*elem;
            size_t count;
        } cunion;
    };
};
struct constraintelem {
    enum N_types N_type;
    union {
        struct {
            intconstant* min;
            intconstant* max;
        }
        range;
        intconstant intvalue;
    };
};
struct intconstraint {
    enum N_types N_type;
    union {
        constraintelem single;
        struct {
            constraintelem*elem;
            size_t count;
        } set;
        intconstraint*  negate;
    };
};
struct constrainedint {
    intp parser;
    intconstraint* constraint;
}
;
struct transform {
    struct {
        streamidentifier*elem;
        size_t count;
    } left;
    varidentifier cfunction;
    struct {
        parameter*elem;
        size_t count;
    } right;
}
;
struct structparser {
    struct {
        enum N_types N_type;
        union {
            constparser constant;
            struct {
                dependencyidentifier name;
                parser*  parser;
            }
            dependency;
            transform transform;
            struct {
                varidentifier name;
                parser*  parser;
            }
            field;
        };
    }*elem;
    size_t count;
};
struct wrapparser {
    struct {
        constparser*elem;
        size_t count;
    }* constbefore;
    parser*  parser;
    struct {
        constparser*elem;
        size_t count;
    }* constafter;
}
;
struct choiceparser {
    struct {
        constidentifier tag;
        parser*  parser;
    }
    *elem;
    size_t count;
};
struct selectparser {
    dependencyidentifier dep;
    struct {
        struct {
            constidentifier tag;
            intconstant value;
            parser*  parser;
        }
        *elem;
        size_t count;
    } options;
}
;
struct arrayparser {
    enum N_types N_type;
    union {
        parser*  manyone;
        parser*  many;
        struct {
            constparser separator;
            parser*  inner;
        }
        sepbyone;
        struct {
            constparser separator;
            parser*  inner;
        }
        sepby;
    };
};
struct parameter {
    enum N_types N_type;
    union {
        dependencyidentifier pdependency;
        streamidentifier pstream;
    };
};
struct parameterlist {
    parameter*elem;
    size_t count;
};
struct parameterdefinition {
    enum N_types N_type;
    union {
        streamidentifier dstream;
        struct {
            dependencyidentifier name;
            parser*  type;
        }
        ddependency;
    };
};
struct parameterdefinitionlist {
    parameterdefinition*elem;
    size_t count;
};
struct parserinvocation {
    varidentifier name;
    parameterlist* parameters;
}
;
struct parserinner {
    enum N_types N_type;
    union {
        constrainedint integer;
        structparser structure;
        wrapparser wrap;
        choiceparser choice;
        selectparser selectp;
        arrayparser array;
        struct {
            intconstant length;
            parser*  inner;
        }
        fixedarray;
        struct {
            dependencyidentifier length;
            parser*  parser;
        }
        length;
        struct {
            streamidentifier stream;
            parser*  inner;
        }
        apply;
        parser*  optional;
        struct {
            parser* *elem;
            size_t count;
        } nunion;
        parserinvocation ref;
        parserinvocation name;
    };
};
struct parser {
    enum N_types N_type;
    union {
        parserinner paren;
        parserinner pr;
    };
};
struct definition {
    enum N_types N_type;
    union {
        struct {
            varidentifier name;
            parameterdefinitionlist* parameters;
            parser definition;
        }
        parser;
        struct {
            constidentifier name;
            constparser definition;
        }
        constantdef;
        struct {
            enum N_types N_type;
            union {
                struct {
                }
                little;
                struct {
                }
                big;
            };
        } endian;
    };
};
struct grammar {
    definition*elem;
    size_t count;
};



struct NailArenaPool;
typedef struct NailArena_ {
    struct NailArenaPool *current;
    size_t blocksize;
} NailArena ;
extern int NailArena_init(NailArena *arena,size_t blocksize);
extern int NailArena_release(NailArena *arena);
extern void *n_malloc(NailArena *arena, size_t size);
struct NailStream {
    const uint8_t *data;
    size_t size;
    size_t pos;
    signed char bit_offset;
};

typedef struct NailStream NailStream;
typedef size_t NailStreamPos;
static NailStream * NailStream_alloc(NailArena *arena) {
    return (NailStream *)n_malloc(arena, sizeof(NailStream));
}
extern int NailOutStream_init(NailStream *str,size_t siz);
extern void NailOutStream_release(NailStream *str);
const uint8_t * NailOutStream_buffer(NailStream *str,size_t *siz);
extern int NailOutStream_grow(NailStream *stream, size_t count);

#define n_fail(i) __builtin_expect(i,0)
number*parse_number(NailArena *arena, const uint8_t *data, size_t size);
varidentifier*parse_varidentifier(NailArena *arena, const uint8_t *data, size_t size);
constidentifier*parse_constidentifier(NailArena *arena, const uint8_t *data, size_t size);
streamidentifier*parse_streamidentifier(NailArena *arena, const uint8_t *data, size_t size);
dependencyidentifier*parse_dependencyidentifier(NailArena *arena, const uint8_t *data, size_t size);
intconstant*parse_intconstant(NailArena *arena, const uint8_t *data, size_t size);
intp*parse_intp(NailArena *arena, const uint8_t *data, size_t size);
constint*parse_constint(NailArena *arena, const uint8_t *data, size_t size);
arrayvalue*parse_arrayvalue(NailArena *arena, const uint8_t *data, size_t size);
constarray*parse_constarray(NailArena *arena, const uint8_t *data, size_t size);
constfields*parse_constfields(NailArena *arena, const uint8_t *data, size_t size);
constparser*parse_constparser(NailArena *arena, const uint8_t *data, size_t size);
constraintelem*parse_constraintelem(NailArena *arena, const uint8_t *data, size_t size);
intconstraint*parse_intconstraint(NailArena *arena, const uint8_t *data, size_t size);
constrainedint*parse_constrainedint(NailArena *arena, const uint8_t *data, size_t size);
transform*parse_transform(NailArena *arena, const uint8_t *data, size_t size);
structparser*parse_structparser(NailArena *arena, const uint8_t *data, size_t size);
wrapparser*parse_wrapparser(NailArena *arena, const uint8_t *data, size_t size);
choiceparser*parse_choiceparser(NailArena *arena, const uint8_t *data, size_t size);
selectparser*parse_selectparser(NailArena *arena, const uint8_t *data, size_t size);
arrayparser*parse_arrayparser(NailArena *arena, const uint8_t *data, size_t size);
parameter*parse_parameter(NailArena *arena, const uint8_t *data, size_t size);
parameterlist*parse_parameterlist(NailArena *arena, const uint8_t *data, size_t size);
parameterdefinition*parse_parameterdefinition(NailArena *arena, const uint8_t *data, size_t size);
parameterdefinitionlist*parse_parameterdefinitionlist(NailArena *arena, const uint8_t *data, size_t size);
parserinvocation*parse_parserinvocation(NailArena *arena, const uint8_t *data, size_t size);
parserinner*parse_parserinner(NailArena *arena, const uint8_t *data, size_t size);
parser*parse_parser(NailArena *arena, const uint8_t *data, size_t size);
definition*parse_definition(NailArena *arena, const uint8_t *data, size_t size);
grammar*parse_grammar(NailArena *arena, const uint8_t *data, size_t size);

int gen_number(NailArena *tmp_arena,NailStream *out,number * val);
int gen_varidentifier(NailArena *tmp_arena,NailStream *out,varidentifier * val);
int gen_constidentifier(NailArena *tmp_arena,NailStream *out,constidentifier * val);
int gen_streamidentifier(NailArena *tmp_arena,NailStream *out,streamidentifier * val);
int gen_dependencyidentifier(NailArena *tmp_arena,NailStream *out,dependencyidentifier * val);
int gen_WHITE(NailStream* str_current);
int gen_SEPERATOR(NailStream* str_current);
int gen_intconstant(NailArena *tmp_arena,NailStream *out,intconstant * val);
int gen_intp(NailArena *tmp_arena,NailStream *out,intp * val);
int gen_constint(NailArena *tmp_arena,NailStream *out,constint * val);
int gen_arrayvalue(NailArena *tmp_arena,NailStream *out,arrayvalue * val);
int gen_constarray(NailArena *tmp_arena,NailStream *out,constarray * val);
int gen_constfields(NailArena *tmp_arena,NailStream *out,constfields * val);
int gen_constparser(NailArena *tmp_arena,NailStream *out,constparser * val);
int gen_constraintelem(NailArena *tmp_arena,NailStream *out,constraintelem * val);
int gen_intconstraint(NailArena *tmp_arena,NailStream *out,intconstraint * val);
int gen_constrainedint(NailArena *tmp_arena,NailStream *out,constrainedint * val);
int gen_transform(NailArena *tmp_arena,NailStream *out,transform * val);
int gen_structparser(NailArena *tmp_arena,NailStream *out,structparser * val);
int gen_wrapparser(NailArena *tmp_arena,NailStream *out,wrapparser * val);
int gen_choiceparser(NailArena *tmp_arena,NailStream *out,choiceparser * val);
int gen_selectparser(NailArena *tmp_arena,NailStream *out,selectparser * val);
int gen_arrayparser(NailArena *tmp_arena,NailStream *out,arrayparser * val);
int gen_parameter(NailArena *tmp_arena,NailStream *out,parameter * val);
int gen_parameterlist(NailArena *tmp_arena,NailStream *out,parameterlist * val);
int gen_parameterdefinition(NailArena *tmp_arena,NailStream *out,parameterdefinition * val);
int gen_parameterdefinitionlist(NailArena *tmp_arena,NailStream *out,parameterdefinitionlist * val);
int gen_parserinvocation(NailArena *tmp_arena,NailStream *out,parserinvocation * val);
int gen_parserinner(NailArena *tmp_arena,NailStream *out,parserinner * val);
int gen_parser(NailArena *tmp_arena,NailStream *out,parser * val);
int gen_definition(NailArena *tmp_arena,NailStream *out,definition * val);
int gen_grammar(NailArena *tmp_arena,NailStream *out,grammar * val);


