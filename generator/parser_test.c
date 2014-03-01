#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
typedef int32_t pos;
typedef struct {
    pos *trace;
    pos capacity,iter,grow;
} n_trace;
uint64_t read_unsigned_bits(const char *data, pos pos, unsigned count) {
    uint64_t retval = 0;
    unsigned int out_idx=0;
    //TODO: Implement little endian too
    //Count LSB to MSB
    while(count>0) {
        if((pos & 7) == 0 && (count &7) ==0) {
            retval|= data[pos >> 3] << out_idx;
            out_idx+=8;
            pos += 8;
            count-=8;
        }
        else {
            assert("BAM!");
            exit(0);
        }
    }
    return retval;
}
#define BITSLICE(x, off, len) read_unsigned_bits(x,off,len)
/* trace is a minimalistic representation of the AST. Many parsers add a count, choice parsers add
 * two pos parameters (which choice was taken and where in the trace it begins)
 * const parsers emit a new input position
*/
typedef struct {
    pos position;
    pos parser;
    pos result;
} n_hash;

typedef struct {
//        p_hash *memo;
    unsigned lg_size; // How large is the hashtable - make it a power of two
} n_hashtable;

static int  n_trace_init(n_trace *out,pos size,pos grow) {
    if(size <= 1) {
        return 0;
    }
    out->trace = malloc(size * sizeof(pos));
    if(!out) {
        return 0;
    }
    out->capacity = size -1 ;
    out->iter = 0;
    if(grow < 16) { // Grow needs to be at least 2, but small grow makes no sense
        grow = 16;
    }
    out->grow = grow;
    return 1;
}
static int n_trace_grow(n_trace *out, int space) {
    if(out->capacity - space>= out->iter) {
        return 0;
    }

    pos * new_ptr= realloc(out->trace, out->capacity + out->grow);
    if(!new_ptr) {
        return 1;
    }
    out->trace = new_ptr;
    out->capacity += out->grow;
    return 0;
}
static pos n_tr_memo_optional(n_trace *trace) {
    if(n_trace_grow(trace,1))
        return -1;
    trace->trace[trace->iter] = 0xFFFFFFFD;
    return trace->iter++;
}
static void n_tr_optional_succeed(n_trace * trace, pos where) {
    trace->trace[where] = -1;
}
static void n_tr_optional_fail(n_trace * trace, pos where) {
    trace->trace[where] = trace->iter;
}
static pos n_tr_memo_many(n_trace *trace) {
    if(n_trace_grow(trace,2))
        return -1;
    trace->trace[trace->iter] = 0xFFFFFFFE;
    trace->trace[trace->iter+1] = 0xEEEEEEEF;
    trace->iter +=2;
    return trace->iter-2;

}
static void n_tr_write_many(n_trace *trace, pos where, pos count) {
    trace->trace[where] = count;
    trace->trace[where+1] = trace->iter;
    fprintf(stderr,"%d = many %d %d\n", where, count,trace->iter);
}

static pos n_tr_begin_choice(n_trace *trace) {
    if(n_trace_grow(trace,2))
        return -1;

    //Debugging values
    trace->trace[trace->iter] = 0xFFFFFFFF;
    trace->trace[trace->iter+1] = 0xEEEEEEEE;
    trace->iter+= 2;
    return trace->iter - 2;
}
static pos n_tr_memo_choice(n_trace *trace) {
    return trace->iter;
}
static void n_tr_pick_choice(n_trace *trace, pos where, pos which_choice, pos  choice_begin) {
    trace->trace[where] = which_choice;
    trace->trace[where + 1] = choice_begin;
    fprintf(stderr,"%d = pick %d %d\n",where, which_choice,choice_begin);
}
static int n_tr_const(n_trace *trace,pos newoff) {
    if(n_trace_grow(trace,1))
        return -1;
    fprintf(stderr,"%d = const %d \n",trace->iter, newoff);
    trace->trace[trace->iter++] = newoff;
    return 0;
}

typedef struct NailArena_ {} NailArena ;
//Returns the pointer where the taken choice is supposed to go.

#define parser_fail(i) __builtin_expect(i<0,0)



#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef struct number number;
typedef struct varidentifier varidentifier;
typedef struct constidentifier constidentifier;
typedef struct contextidentifier contextidentifier;
typedef struct intconstant intconstant;
typedef struct intp intp;
typedef struct constint constint;
typedef struct arrayvalue arrayvalue;
typedef struct constarray constarray;
typedef struct constfields constfields;
typedef struct constparser constparser;
typedef struct intconstraint intconstraint;
typedef struct constrainedint constrainedint;
typedef struct structparser structparser;
typedef struct wrapparser wrapparser;
typedef struct choiceparser choiceparser;
typedef struct arrayparser arrayparser;
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
struct contextidentifier {
    uint8_t*elem;
    size_t count;
};
struct intconstant {
    enum  {ASCII,NUMBER} N_type;
    union {
        struct {
            enum  {ESCAPE,DIRECT} N_type;
            union {
                uint8_t ESCAPE;
                uint8_t DIRECT;
            };
        } ASCII;
        number NUMBER;
    };
};
struct intp {
    enum  {UNSIGNED,SIGNED} N_type;
    union {
        struct {
            uint8_t*elem;
            size_t count;
        } UNSIGNED;
        struct {
            uint8_t*elem;
            size_t count;
        } SIGNED;
    };
};
struct constint {
    intp parser;
    intconstant value;
}
;
struct arrayvalue {
    enum  {STRING,VALUES} N_type;
    union {
        struct {
            uint8_t*elem;
            size_t count;
        } STRING;
        struct {
            intconstant*elem;
            size_t count;
        } VALUES;
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
    enum  {CARRAY,CREPEAT,CINT,CREF,CSTRUCT,CUNION} N_type;
    union {
        constarray CARRAY;
        constparser*  CREPEAT;
        constint CINT;
        constidentifier CREF;
        constfields CSTRUCT;
        struct {
            constparser*elem;
            size_t count;
        } CUNION;
    };
};
struct intconstraint {
    enum  {RANGE,SET,NEGATE} N_type;
    union {
        struct {
            intconstant* min;
            intconstant* max;
        }
        RANGE;
        struct {
            intconstant*elem;
            size_t count;
        } SET;
        intconstraint*  NEGATE;
    };
};
struct constrainedint {
    intp parser;
    intconstraint* constraint;
}
;
struct structparser {
    struct {
        enum  {CONSTANT,CTXT,FIELD} N_type;
        union {
            constparser CONSTANT;
            struct {
                contextidentifier name;
                parser*  parser;
            }
            CTXT;
            struct {
                varidentifier name;
                parser*  parser;
            }
            FIELD;
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
struct arrayparser {
    enum  {MANYONE,MANY,SEPBYONE,SEPBY} N_type;
    union {
        parser*  MANYONE;
        parser*  MANY;
        struct {
            constparser separator;
            parser*  inner;
        }
        SEPBYONE;
        struct {
            constparser separator;
            parser*  inner;
        }
        SEPBY;
    };
};
struct parserinner {
    enum  {INT,STRUCT,WRAP,CHOICE,ARRAY,OPTIONAL,UNION,REF,NAME} N_type;
    union {
        constrainedint INT;
        structparser STRUCT;
        wrapparser WRAP;
        choiceparser CHOICE;
        arrayparser ARRAY;
        parser*  OPTIONAL;
        struct {
            parser**elem;
            size_t count;
        } UNION;
        varidentifier REF;
        varidentifier NAME;
    };
};
struct parser {
    enum  {PAREN,PR} N_type;
    union {
        parserinner PAREN;
        parserinner PR;
    };
};
struct definition {
    enum  {PARSER,CONST} N_type;
    union {
        struct {
            varidentifier name;
            parser definition;
        }
        PARSER;
        struct {
            constidentifier name;
            constparser definition;
        }
        CONST;
    };
};
struct grammar {
    definition*elem;
    size_t count;
};

static pos packrat_number(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_varidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_contextidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_WHITE(const char *data, pos off, pos max);
static pos packrat_SEPERATOR(const char *data, pos off, pos max);
static pos packrat_intconstant(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_intp(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_arrayvalue(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constarray(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constfields(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_intconstraint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_constrainedint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_structparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_wrapparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_choiceparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_arrayparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_parserinner(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_parser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_definition(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_grammar(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max);
static pos packrat_number(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_0:
        if(off + 8> max) {
            goto fail_repeat_0;
        }
        {
            uint64_t val = read_unsigned_bits(data,off,8);
            if(val>'9'|| val < '0') {
                goto fail_repeat_0;
            }
        }
        off +=8;
        count++;
        goto succ_repeat_0;
fail_repeat_0:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return off;
fail:
    return -1;
}
static pos packrat_varidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_1:
        if(off + 8> max) {
            goto fail_repeat_1;
        }
        {
            uint64_t val = read_unsigned_bits(data,off,8);
            if(val>'z'|| val < 'a') {
                goto fail_repeat_1;
            }
        }
        off +=8;
        count++;
        goto succ_repeat_1;
fail_repeat_1:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return off;
fail:
    return -1;
}
static pos packrat_constidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_2:
        if(off + 8> max) {
            goto fail_repeat_2;
        }
        {
            uint64_t val = read_unsigned_bits(data,off,8);
            if(val>'Z'|| val < 'A') {
                goto fail_repeat_2;
            }
        }
        off +=8;
        count++;
        goto succ_repeat_2;
fail_repeat_2:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return off;
fail:
    return -1;
}
static pos packrat_contextidentifier(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '@') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_3:
        if(off + 8> max) {
            goto fail_repeat_3;
        }
        {
            uint64_t val = read_unsigned_bits(data,off,8);
            if(val>'z'|| val < 'a') {
                goto fail_repeat_3;
            }
        }
        off +=8;
        count++;
        goto succ_repeat_3;
fail_repeat_3:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return off;
fail:
    return -1;
}
static pos packrat_WHITE(const char *data, pos off, pos max) {
constmany_0_repeat:
    {
        pos backtrack = off;
        if(off + 8> max) {
            goto cunion_1_1;
        }
        if( read_unsigned_bits(data,off,8)!= ' ') {
            goto cunion_1_1;
        }
        off += 8;
        goto cunion_1_succ;
cunion_1_1:
        off = backtrack;
        if(off + 8> max) {
            goto cunion_1_2;
        }
        if( read_unsigned_bits(data,off,8)!= '\n') {
            goto cunion_1_2;
        }
        off += 8;
        goto cunion_1_succ;
cunion_1_2:
        off = backtrack;
        goto constmany_1_end;
cunion_1_succ:
        ;
    }
    goto constmany_0_repeat;
constmany_1_end:
    return off;
fail:
    return -1;
}
static pos packrat_SEPERATOR(const char *data, pos off, pos max) {
constmany_2_repeat:
    if(off + 8> max) {
        goto constmany_3_end;
    }
    if( read_unsigned_bits(data,off,8)!= ' ') {
        goto constmany_3_end;
    }
    off += 8;
    goto constmany_2_repeat;
constmany_3_end:
constmany_4_repeat:
    {
        pos backtrack = off;
        if(off + 8> max) {
            goto cunion_2_1;
        }
        if( read_unsigned_bits(data,off,8)!= '\n') {
            goto cunion_2_1;
        }
        off += 8;
        goto cunion_2_succ;
cunion_2_1:
        off = backtrack;
        if(off + 8> max) {
            goto cunion_2_2;
        }
        if( read_unsigned_bits(data,off,8)!= ';') {
            goto cunion_2_2;
        }
        off += 8;
        goto cunion_2_succ;
cunion_2_2:
        off = backtrack;
        goto constmany_5_end;
cunion_2_succ:
        ;
    }
    goto constmany_4_repeat;
constmany_5_end:
    return off;
fail:
    return -1;
}
static pos packrat_intconstant(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(off + 8> max) {
            goto choice_3_ASCII_out;
        }
        if( read_unsigned_bits(data,off,8)!= '\'') {
            goto choice_3_ASCII_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_3_ASCII_out;
        }
        {   pos backtrack = off;
            pos choice_begin = n_tr_begin_choice(trace);
            pos choice;
            if(parser_fail(choice_begin)) {
                goto choice_3_ASCII_out;
            }
            choice = n_tr_memo_choice(trace);
            if(off + 8> max) {
                goto choice_4_ESCAPE_out;
            }
            if( read_unsigned_bits(data,off,8)!= '\\') {
                goto choice_4_ESCAPE_out;
            }
            off += 8;
            if(parser_fail(n_tr_const(trace,off))) {
                goto choice_4_ESCAPE_out;
            }
            if(off + 8> max) {
                goto choice_4_ESCAPE_out;
            }
            off +=8;
            n_tr_pick_choice(trace,choice_begin,ESCAPE,choice);
            goto choice_4_succ;
choice_4_ESCAPE_out:
            off = backtrack;
            choice = n_tr_memo_choice(trace);
            if(off + 8> max) {
                goto choice_4_DIRECT_out;
            }
            off +=8;
            n_tr_pick_choice(trace,choice_begin,DIRECT,choice);
            goto choice_4_succ;
choice_4_DIRECT_out:
            off = backtrack;
            goto choice_3_ASCII_out;
choice_4_succ:
            ;
        }
        if(off + 8> max) {
            goto choice_3_ASCII_out;
        }
        if( read_unsigned_bits(data,off,8)!= '\'') {
            goto choice_3_ASCII_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_3_ASCII_out;
        }
        n_tr_pick_choice(trace,choice_begin,ASCII,choice);
        goto choice_3_succ;
choice_3_ASCII_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_number(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_3_NUMBER_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,NUMBER,choice);
        goto choice_3_succ;
choice_3_NUMBER_out:
        off = backtrack;
        goto fail;
choice_3_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_intp(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_5_UNSIGNED_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_5_UNSIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'u') {
            goto choice_5_UNSIGNED_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_5_UNSIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'i') {
            goto choice_5_UNSIGNED_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_5_UNSIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_5_UNSIGNED_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_5_UNSIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 't') {
            goto choice_5_UNSIGNED_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_5_UNSIGNED_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_4:
            if(off + 8> max) {
                goto fail_repeat_4;
            }
            {
                uint64_t val = read_unsigned_bits(data,off,8);
                if(val>'9'|| val < '0') {
                    goto fail_repeat_4;
                }
            }
            off +=8;
            count++;
            goto succ_repeat_4;
fail_repeat_4:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_5_UNSIGNED_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,UNSIGNED,choice);
        goto choice_5_succ;
choice_5_UNSIGNED_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_5_SIGNED_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_5_SIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'i') {
            goto choice_5_SIGNED_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_5_SIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_5_SIGNED_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_5_SIGNED_out;
        }
        if( read_unsigned_bits(data,off,8)!= 't') {
            goto choice_5_SIGNED_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_5_SIGNED_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_5:
            if(off + 8> max) {
                goto fail_repeat_5;
            }
            {
                uint64_t val = read_unsigned_bits(data,off,8);
                if(val>'9'|| val < '0') {
                    goto fail_repeat_5;
                }
            }
            off +=8;
            count++;
            goto succ_repeat_5;
fail_repeat_5:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_5_SIGNED_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,SIGNED,choice);
        goto choice_5_succ;
choice_5_SIGNED_out:
        off = backtrack;
        goto fail;
choice_5_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_constint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    i = packrat_intp(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '=') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    i = packrat_intconstant(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    return off;
fail:
    return -1;
}
static pos packrat_arrayvalue(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_6_STRING_out;
            }
            off = ext;
        }
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_6_STRING_out;
        }
        if(off + 8> max) {
            goto choice_6_STRING_out;
        }
        if( read_unsigned_bits(data,off,8)!= '"') {
            goto choice_6_STRING_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_6_STRING_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_6:
            if(off + 8> max) {
                goto fail_repeat_6;
            }
            {
                uint64_t val = read_unsigned_bits(data,off,8);
                if(!(val != '"')) {
                    goto fail_repeat_6;
                }
            }
            off +=8;
            count++;
            goto succ_repeat_6;
fail_repeat_6:
            n_tr_write_many(trace,many,count);
        }
        if(off + 8> max) {
            goto choice_6_STRING_out;
        }
        if( read_unsigned_bits(data,off,8)!= '"') {
            goto choice_6_STRING_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_6_STRING_out;
        }
        n_tr_pick_choice(trace,choice_begin,STRING,choice);
        goto choice_6_succ;
choice_6_STRING_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_6_VALUES_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_6_VALUES_out;
        }
        if( read_unsigned_bits(data,off,8)!= '[') {
            goto choice_6_VALUES_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_6_VALUES_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_7:
            i = packrat_intconstant(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_7;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_7;
fail_repeat_7:
            n_tr_write_many(trace,many,count);
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_6_VALUES_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_6_VALUES_out;
        }
        if( read_unsigned_bits(data,off,8)!= ']') {
            goto choice_6_VALUES_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_6_VALUES_out;
        }
        n_tr_pick_choice(trace,choice_begin,VALUES,choice);
        goto choice_6_succ;
choice_6_VALUES_out:
        off = backtrack;
        goto fail;
choice_6_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_constarray(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'm') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'a') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'n') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'y') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= ' ') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    i = packrat_intp(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '=') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    i = packrat_arrayvalue(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    return off;
fail:
    return -1;
}
static pos packrat_constfields(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_8:
        if(count>0) {
            {
                pos ext = packrat_SEPERATOR(data,off,max);
                if(ext < 0) {
                    goto fail_repeat_8;
                }
                off = ext;
            }
            if(parser_fail(n_tr_const(trace,off))) {
                goto fail_repeat_8;
            }
        }
        i = packrat_constparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto fail_repeat_8;
        }
        else {
            off=i;
        }
        count++;
        goto succ_repeat_8;
fail_repeat_8:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return off;
fail:
    return -1;
}
static pos packrat_constparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        i = packrat_constarray(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_7_CARRAY_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CARRAY,choice);
        goto choice_7_succ;
choice_7_CARRAY_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_7_CREPEAT_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_7_CREPEAT_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'm') {
            goto choice_7_CREPEAT_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_7_CREPEAT_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'a') {
            goto choice_7_CREPEAT_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_7_CREPEAT_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_7_CREPEAT_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_7_CREPEAT_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'y') {
            goto choice_7_CREPEAT_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_7_CREPEAT_out;
        }
        if( read_unsigned_bits(data,off,8)!= ' ') {
            goto choice_7_CREPEAT_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_7_CREPEAT_out;
        }
        i = packrat_constparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_7_CREPEAT_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CREPEAT,choice);
        goto choice_7_succ;
choice_7_CREPEAT_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_constint(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_7_CINT_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CINT,choice);
        goto choice_7_succ;
choice_7_CINT_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_constidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_7_CREF_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CREF,choice);
        goto choice_7_succ;
choice_7_CREF_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_7_CSTRUCT_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_7_CSTRUCT_out;
        }
        if( read_unsigned_bits(data,off,8)!= '{') {
            goto choice_7_CSTRUCT_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_7_CSTRUCT_out;
        }
        i = packrat_constfields(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_7_CSTRUCT_out;
        }
        else {
            off=i;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_7_CSTRUCT_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_7_CSTRUCT_out;
        }
        if( read_unsigned_bits(data,off,8)!= '}') {
            goto choice_7_CSTRUCT_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_7_CSTRUCT_out;
        }
        n_tr_pick_choice(trace,choice_begin,CSTRUCT,choice);
        goto choice_7_succ;
choice_7_CSTRUCT_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_9:
            {
                pos ext = packrat_WHITE(data,off,max);
                if(ext < 0) {
                    goto fail_repeat_9;
                }
                off = ext;
            }
            if(off + 8> max) {
                goto fail_repeat_9;
            }
            if( read_unsigned_bits(data,off,8)!= '|') {
                goto fail_repeat_9;
            }
            off += 8;
            if(off + 8> max) {
                goto fail_repeat_9;
            }
            if( read_unsigned_bits(data,off,8)!= '|') {
                goto fail_repeat_9;
            }
            off += 8;
            if(parser_fail(n_tr_const(trace,off))) {
                goto fail_repeat_9;
            }
            i = packrat_constparser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_9;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_9;
fail_repeat_9:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_7_CUNION_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,CUNION,choice);
        goto choice_7_succ;
choice_7_CUNION_out:
        off = backtrack;
        goto fail;
choice_7_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_intconstraint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_optional(trace);
            i = packrat_intconstant(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_optional_10;
            }
            else {
                off=i;
            }
            n_tr_optional_succeed(trace,many);
            goto succ_optional_10;
fail_optional_10:
            n_tr_optional_fail(trace,many);
succ_optional_10:
            ;
        }{
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_8_RANGE_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_8_RANGE_out;
        }
        if( read_unsigned_bits(data,off,8)!= '.') {
            goto choice_8_RANGE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_8_RANGE_out;
        }
        if( read_unsigned_bits(data,off,8)!= '.') {
            goto choice_8_RANGE_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_8_RANGE_out;
        }
        {
            pos many = n_tr_memo_optional(trace);
            i = packrat_intconstant(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_optional_11;
            }
            else {
                off=i;
            }
            n_tr_optional_succeed(trace,many);
            goto succ_optional_11;
fail_optional_11:
            n_tr_optional_fail(trace,many);
succ_optional_11:
            ;
        }
        n_tr_pick_choice(trace,choice_begin,RANGE,choice);
        goto choice_8_succ;
choice_8_RANGE_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_8_SET_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_8_SET_out;
        }
        if( read_unsigned_bits(data,off,8)!= '[') {
            goto choice_8_SET_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_8_SET_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_12:
            if(count>0) {
                if(off + 8> max) {
                    goto fail_repeat_12;
                }
                if( read_unsigned_bits(data,off,8)!= ',') {
                    goto fail_repeat_12;
                }
                off += 8;
                if(parser_fail(n_tr_const(trace,off))) {
                    goto fail_repeat_12;
                }
            }
            i = packrat_intconstant(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_12;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_12;
fail_repeat_12:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_8_SET_out;
            }
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_8_SET_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_8_SET_out;
        }
        if( read_unsigned_bits(data,off,8)!= ']') {
            goto choice_8_SET_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_8_SET_out;
        }
        n_tr_pick_choice(trace,choice_begin,SET,choice);
        goto choice_8_succ;
choice_8_SET_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_8_NEGATE_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_8_NEGATE_out;
        }
        if( read_unsigned_bits(data,off,8)!= '!') {
            goto choice_8_NEGATE_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_8_NEGATE_out;
        }
        i = packrat_intconstraint(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_8_NEGATE_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,NEGATE,choice);
        goto choice_8_succ;
choice_8_NEGATE_out:
        off = backtrack;
        goto fail;
choice_8_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_constrainedint(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    i = packrat_intp(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    {
        pos many = n_tr_memo_optional(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto fail_optional_13;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto fail_optional_13;
        }
        if( read_unsigned_bits(data,off,8)!= '|') {
            goto fail_optional_13;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto fail_optional_13;
        }
        i = packrat_intconstraint(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto fail_optional_13;
        }
        else {
            off=i;
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_13;
fail_optional_13:
        n_tr_optional_fail(trace,many);
succ_optional_13:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_structparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '{') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_14:
        if(count>0) {
            {
                pos ext = packrat_SEPERATOR(data,off,max);
                if(ext < 0) {
                    goto fail_repeat_14;
                }
                off = ext;
            }
            if(parser_fail(n_tr_const(trace,off))) {
                goto fail_repeat_14;
            }
        }
        {   pos backtrack = off;
            pos choice_begin = n_tr_begin_choice(trace);
            pos choice;
            if(parser_fail(choice_begin)) {
                goto fail_repeat_14;
            }
            choice = n_tr_memo_choice(trace);
            i = packrat_constparser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto choice_9_CONSTANT_out;
            }
            else {
                off=i;
            }
            n_tr_pick_choice(trace,choice_begin,CONSTANT,choice);
            goto choice_9_succ;
choice_9_CONSTANT_out:
            off = backtrack;
            choice = n_tr_memo_choice(trace);
            i = packrat_contextidentifier(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto choice_9_CTXT_out;
            }
            else {
                off=i;
            }
            i = packrat_parser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto choice_9_CTXT_out;
            }
            else {
                off=i;
            }
            n_tr_pick_choice(trace,choice_begin,CTXT,choice);
            goto choice_9_succ;
choice_9_CTXT_out:
            off = backtrack;
            choice = n_tr_memo_choice(trace);
            i = packrat_varidentifier(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto choice_9_FIELD_out;
            }
            else {
                off=i;
            }
            i = packrat_parser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto choice_9_FIELD_out;
            }
            else {
                off=i;
            }
            n_tr_pick_choice(trace,choice_begin,FIELD,choice);
            goto choice_9_succ;
choice_9_FIELD_out:
            off = backtrack;
            goto fail_repeat_14;
choice_9_succ:
            ;
        }
        count++;
        goto succ_repeat_14;
fail_repeat_14:
        n_tr_write_many(trace,many,count);
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '}') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    return off;
fail:
    return -1;
}
static pos packrat_wrapparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '<') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_optional(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_16:
            if(count>0) {
                {
                    pos ext = packrat_SEPERATOR(data,off,max);
                    if(ext < 0) {
                        goto fail_repeat_16;
                    }
                    off = ext;
                }
                if(parser_fail(n_tr_const(trace,off))) {
                    goto fail_repeat_16;
                }
            }
            i = packrat_constparser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_16;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_16;
fail_repeat_16:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto fail_optional_15;
            }
        }
        {
            pos ext = packrat_SEPERATOR(data,off,max);
            if(ext < 0) {
                goto fail_optional_15;
            }
            off = ext;
        }
        if(parser_fail(n_tr_const(trace,off))) {
            goto fail_optional_15;
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_15;
fail_optional_15:
        n_tr_optional_fail(trace,many);
succ_optional_15:
        ;
    }
    i = packrat_parser(tmp, trace,data,off,max);
    if(parser_fail(i)) {
        goto fail;
    }
    else {
        off=i;
    }
    {
        pos many = n_tr_memo_optional(trace);
        {
            pos ext = packrat_SEPERATOR(data,off,max);
            if(ext < 0) {
                goto fail_optional_17;
            }
            off = ext;
        }
        if(parser_fail(n_tr_const(trace,off))) {
            goto fail_optional_17;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_18:
            if(count>0) {
                {
                    pos ext = packrat_SEPERATOR(data,off,max);
                    if(ext < 0) {
                        goto fail_repeat_18;
                    }
                    off = ext;
                }
                if(parser_fail(n_tr_const(trace,off))) {
                    goto fail_repeat_18;
                }
            }
            i = packrat_constparser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_18;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_18;
fail_repeat_18:
            n_tr_write_many(trace,many,count);
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_17;
fail_optional_17:
        n_tr_optional_fail(trace,many);
succ_optional_17:
        ;
    }{
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '>') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    return off;
fail:
    return -1;
}
static pos packrat_choiceparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'c') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'h') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'o') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'o') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 's') {
        goto fail;
    }
    off += 8;
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= 'e') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '{') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_19:
        i = packrat_constidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto fail_repeat_19;
        }
        else {
            off=i;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto fail_repeat_19;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto fail_repeat_19;
        }
        if( read_unsigned_bits(data,off,8)!= '=') {
            goto fail_repeat_19;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto fail_repeat_19;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto fail_repeat_19;
        }
        else {
            off=i;
        }
        count++;
        goto succ_repeat_19;
fail_repeat_19:
        n_tr_write_many(trace,many,count);
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(off + 8> max) {
        goto fail;
    }
    if( read_unsigned_bits(data,off,8)!= '}') {
        goto fail;
    }
    off += 8;
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    return off;
fail:
    return -1;
}
static pos packrat_arrayparser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_10_MANYONE_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_10_MANYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'm') {
            goto choice_10_MANYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'a') {
            goto choice_10_MANYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_10_MANYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'y') {
            goto choice_10_MANYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= '1') {
            goto choice_10_MANYONE_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_10_MANYONE_out;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_MANYONE_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,MANYONE,choice);
        goto choice_10_succ;
choice_10_MANYONE_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_10_MANY_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_10_MANY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'm') {
            goto choice_10_MANY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'a') {
            goto choice_10_MANY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_10_MANY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_MANY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'y') {
            goto choice_10_MANY_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_10_MANY_out;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_MANY_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,MANY,choice);
        goto choice_10_succ;
choice_10_MANY_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_10_SEPBYONE_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 's') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'e') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'p') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'B') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'y') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBYONE_out;
        }
        if( read_unsigned_bits(data,off,8)!= '1') {
            goto choice_10_SEPBYONE_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_10_SEPBYONE_out;
        }
        i = packrat_constparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_SEPBYONE_out;
        }
        else {
            off=i;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_SEPBYONE_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,SEPBYONE,choice);
        goto choice_10_succ;
choice_10_SEPBYONE_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_10_SEPBY_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_10_SEPBY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 's') {
            goto choice_10_SEPBY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'e') {
            goto choice_10_SEPBY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'p') {
            goto choice_10_SEPBY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'B') {
            goto choice_10_SEPBY_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_10_SEPBY_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'y') {
            goto choice_10_SEPBY_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_10_SEPBY_out;
        }
        i = packrat_constparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_SEPBY_out;
        }
        else {
            off=i;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_10_SEPBY_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,SEPBY,choice);
        goto choice_10_succ;
choice_10_SEPBY_out:
        off = backtrack;
        goto fail;
choice_10_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_parserinner(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        i = packrat_constrainedint(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_INT_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,INT,choice);
        goto choice_11_succ;
choice_11_INT_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_structparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_STRUCT_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,STRUCT,choice);
        goto choice_11_succ;
choice_11_STRUCT_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_wrapparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_WRAP_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,WRAP,choice);
        goto choice_11_succ;
choice_11_WRAP_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_choiceparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_CHOICE_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CHOICE,choice);
        goto choice_11_succ;
choice_11_CHOICE_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_arrayparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_ARRAY_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,ARRAY,choice);
        goto choice_11_succ;
choice_11_ARRAY_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_11_OPTIONAL_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'o') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'p') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 't') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'i') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'o') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'n') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'a') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= 'l') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(off + 8> max) {
            goto choice_11_OPTIONAL_out;
        }
        if( read_unsigned_bits(data,off,8)!= ' ') {
            goto choice_11_OPTIONAL_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_11_OPTIONAL_out;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_OPTIONAL_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,OPTIONAL,choice);
        goto choice_11_succ;
choice_11_OPTIONAL_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_20:
            {
                pos ext = packrat_WHITE(data,off,max);
                if(ext < 0) {
                    goto fail_repeat_20;
                }
                off = ext;
            }
            if(off + 8> max) {
                goto fail_repeat_20;
            }
            if( read_unsigned_bits(data,off,8)!= '|') {
                goto fail_repeat_20;
            }
            off += 8;
            if(off + 8> max) {
                goto fail_repeat_20;
            }
            if( read_unsigned_bits(data,off,8)!= '|') {
                goto fail_repeat_20;
            }
            off += 8;
            if(parser_fail(n_tr_const(trace,off))) {
                goto fail_repeat_20;
            }
            i = packrat_parser(tmp, trace,data,off,max);
            if(parser_fail(i)) {
                goto fail_repeat_20;
            }
            else {
                off=i;
            }
            count++;
            goto succ_repeat_20;
fail_repeat_20:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_11_UNION_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,UNION,choice);
        goto choice_11_succ;
choice_11_UNION_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_11_REF_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_11_REF_out;
        }
        if( read_unsigned_bits(data,off,8)!= '*') {
            goto choice_11_REF_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_11_REF_out;
        }
        i = packrat_varidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_REF_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,REF,choice);
        goto choice_11_succ;
choice_11_REF_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_varidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_11_NAME_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,NAME,choice);
        goto choice_11_succ;
choice_11_NAME_out:
        off = backtrack;
        goto fail;
choice_11_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_parser(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_12_PAREN_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_12_PAREN_out;
        }
        if( read_unsigned_bits(data,off,8)!= '(') {
            goto choice_12_PAREN_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_12_PAREN_out;
        }
        i = packrat_parserinner(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_12_PAREN_out;
        }
        else {
            off=i;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_12_PAREN_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_12_PAREN_out;
        }
        if( read_unsigned_bits(data,off,8)!= ')') {
            goto choice_12_PAREN_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_12_PAREN_out;
        }
        n_tr_pick_choice(trace,choice_begin,PAREN,choice);
        goto choice_12_succ;
choice_12_PAREN_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_parserinner(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_12_PR_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,PR,choice);
        goto choice_12_succ;
choice_12_PR_out:
        off = backtrack;
        goto fail;
choice_12_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_definition(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {   pos backtrack = off;
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        i = packrat_varidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_13_PARSER_out;
        }
        else {
            off=i;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_13_PARSER_out;
            }
            off = ext;
        }
        if(off + 8> max) {
            goto choice_13_PARSER_out;
        }
        if( read_unsigned_bits(data,off,8)!= '=') {
            goto choice_13_PARSER_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_13_PARSER_out;
        }
        i = packrat_parser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_13_PARSER_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,PARSER,choice);
        goto choice_13_succ;
choice_13_PARSER_out:
        off = backtrack;
        choice = n_tr_memo_choice(trace);
        i = packrat_constidentifier(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_13_CONST_out;
        }
        else {
            off=i;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_13_CONST_out;
            }
            off = ext;
        }
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_13_CONST_out;
        }
        if(off + 8> max) {
            goto choice_13_CONST_out;
        }
        if( read_unsigned_bits(data,off,8)!= '=') {
            goto choice_13_CONST_out;
        }
        off += 8;
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_13_CONST_out;
        }
        {
            pos ext = packrat_WHITE(data,off,max);
            if(ext < 0) {
                goto choice_13_CONST_out;
            }
            off = ext;
        }
        if(parser_fail(n_tr_const(trace,off))) {
            goto choice_13_CONST_out;
        }
        i = packrat_constparser(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto choice_13_CONST_out;
        }
        else {
            off=i;
        }
        n_tr_pick_choice(trace,choice_begin,CONST,choice);
        goto choice_13_succ;
choice_13_CONST_out:
        off = backtrack;
        goto fail;
choice_13_succ:
        ;
    }
    return off;
fail:
    return -1;
}
static pos packrat_grammar(NailArena *tmp,n_trace *trace, const char *data, pos off, pos max) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_21:
        i = packrat_definition(tmp, trace,data,off,max);
        if(parser_fail(i)) {
            goto fail_repeat_21;
        }
        else {
            off=i;
        }
        count++;
        goto succ_repeat_21;
fail_repeat_21:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    {
        pos ext = packrat_WHITE(data,off,max);
        if(ext < 0) {
            goto fail;
        }
        off = ext;
    }
    if(parser_fail(n_tr_const(trace,off))) {
        goto fail;
    }
    return off;
fail:
    return -1;
}

static pos bind_number(number*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_varidentifier(varidentifier*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constidentifier(constidentifier*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_contextidentifier(contextidentifier*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_intconstant(intconstant*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_intp(intp*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constint(constint*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_arrayvalue(arrayvalue*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constarray(constarray*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constfields(constfields*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constparser(constparser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_intconstraint(intconstraint*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_constrainedint(constrainedint*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_structparser(structparser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_wrapparser(wrapparser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_choiceparser(choiceparser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_arrayparser(arrayparser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_parserinner(parserinner*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_parser(parser*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_definition(definition*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_grammar(grammar*out,const char *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_number(number*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            out->elem[i]=read_unsigned_bits(data,off,8);
            off += 8;
        }
        tr = trace_begin + save;
    }*trace = tr;
    return off;
}
static pos bind_varidentifier(varidentifier*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            out->elem[i]=read_unsigned_bits(data,off,8);
            off += 8;
        }
        tr = trace_begin + save;
    }*trace = tr;
    return off;
}
static pos bind_constidentifier(constidentifier*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            out->elem[i]=read_unsigned_bits(data,off,8);
            off += 8;
        }
        tr = trace_begin + save;
    }*trace = tr;
    return off;
}
static pos bind_contextidentifier(contextidentifier*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            out->elem[i]=read_unsigned_bits(data,off,8);
            off += 8;
        }
        tr = trace_begin + save;
    }*trace = tr;
    return off;
}
static pos bind_intconstant(intconstant*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case ASCII:
        tr = trace_begin + *tr;
        out->N_type= ASCII;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
        switch(*(tr++)) {
        case ESCAPE:
            tr = trace_begin + *tr;
            out->ASCII.N_type= ESCAPE;
            fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
            off  = *tr;
            tr++;
            out->ASCII.ESCAPE=read_unsigned_bits(data,off,8);
            off += 8;
            break;
        case DIRECT:
            tr = trace_begin + *tr;
            out->ASCII.N_type= DIRECT;
            out->ASCII.DIRECT=read_unsigned_bits(data,off,8);
            off += 8;
            break;
        default:
            assert("BUG");
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    case NUMBER:
        tr = trace_begin + *tr;
        out->N_type= NUMBER;
        off = bind_number(&out->NUMBER, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_intp(intp*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case UNSIGNED:
        tr = trace_begin + *tr;
        out->N_type= UNSIGNED;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->UNSIGNED.count=*(tr++);
            save = *(tr++);
            out->UNSIGNED.elem= malloc(out->UNSIGNED.count* sizeof(*out->UNSIGNED.elem));
            if(!out->UNSIGNED.elem) {
                return 0;
            }
            for(pos i=0; i<out->UNSIGNED.count; i++) {
                out->UNSIGNED.elem[i]=read_unsigned_bits(data,off,8);
                off += 8;
            }
            tr = trace_begin + save;
        }
        break;
    case SIGNED:
        tr = trace_begin + *tr;
        out->N_type= SIGNED;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->SIGNED.count=*(tr++);
            save = *(tr++);
            out->SIGNED.elem= malloc(out->SIGNED.count* sizeof(*out->SIGNED.elem));
            if(!out->SIGNED.elem) {
                return 0;
            }
            for(pos i=0; i<out->SIGNED.count; i++) {
                out->SIGNED.elem[i]=read_unsigned_bits(data,off,8);
                off += 8;
            }
            tr = trace_begin + save;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_constint(constint*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    off = bind_intp(&out->parser, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    off = bind_intconstant(&out->value, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }*trace = tr;
    return off;
}
static pos bind_arrayvalue(arrayvalue*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case STRING:
        tr = trace_begin + *tr;
        out->N_type= STRING;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->STRING.count=*(tr++);
            save = *(tr++);
            out->STRING.elem= malloc(out->STRING.count* sizeof(*out->STRING.elem));
            if(!out->STRING.elem) {
                return 0;
            }
            for(pos i=0; i<out->STRING.count; i++) {
                out->STRING.elem[i]=read_unsigned_bits(data,off,8);
                off += 8;
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    case VALUES:
        tr = trace_begin + *tr;
        out->N_type= VALUES;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->VALUES.count=*(tr++);
            save = *(tr++);
            out->VALUES.elem= malloc(out->VALUES.count* sizeof(*out->VALUES.elem));
            if(!out->VALUES.elem) {
                return 0;
            }
            for(pos i=0; i<out->VALUES.count; i++) {
                off = bind_intconstant(&out->VALUES.elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_constarray(constarray*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    off = bind_intp(&out->parser, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    off = bind_arrayvalue(&out->value, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }*trace = tr;
    return off;
}
static pos bind_constfields(constfields*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            if(i>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                off  = *tr;
                tr++;
            }
            off = bind_constparser(&out->elem[i], data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }*trace = tr;
    return off;
}
static pos bind_constparser(constparser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case CARRAY:
        tr = trace_begin + *tr;
        out->N_type= CARRAY;
        off = bind_constarray(&out->CARRAY, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CREPEAT:
        tr = trace_begin + *tr;
        out->N_type= CREPEAT;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        out->CREPEAT= malloc(sizeof(*out->CREPEAT));
        if(!out->CREPEAT) {
            return -1;
        }
        off = bind_constparser(out->CREPEAT, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CINT:
        tr = trace_begin + *tr;
        out->N_type= CINT;
        off = bind_constint(&out->CINT, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CREF:
        tr = trace_begin + *tr;
        out->N_type= CREF;
        off = bind_constidentifier(&out->CREF, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CSTRUCT:
        tr = trace_begin + *tr;
        out->N_type= CSTRUCT;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_constfields(&out->CSTRUCT, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    case CUNION:
        tr = trace_begin + *tr;
        out->N_type= CUNION;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->CUNION.count=*(tr++);
            save = *(tr++);
            out->CUNION.elem= malloc(out->CUNION.count* sizeof(*out->CUNION.elem));
            if(!out->CUNION.elem) {
                return 0;
            }
            for(pos i=0; i<out->CUNION.count; i++) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                off  = *tr;
                tr++;
                off = bind_constparser(&out->CUNION.elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_intconstraint(intconstraint*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case RANGE:
        tr = trace_begin + *tr;
        out->N_type= RANGE;
        if(*tr<0) { /*OPTIONAL*/
            tr++;
            out->RANGE.min= malloc(sizeof(*out->RANGE.min));
            if(!out->RANGE.min) return -1;
            off = bind_intconstant(&out->RANGE.min[0], data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
        }
        else {
            tr = trace_begin + *tr;
            out->RANGE.min= NULL;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        if(*tr<0) { /*OPTIONAL*/
            tr++;
            out->RANGE.max= malloc(sizeof(*out->RANGE.max));
            if(!out->RANGE.max) return -1;
            off = bind_intconstant(&out->RANGE.max[0], data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
        }
        else {
            tr = trace_begin + *tr;
            out->RANGE.max= NULL;
        }
        break;
    case SET:
        tr = trace_begin + *tr;
        out->N_type= SET;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->SET.count=*(tr++);
            save = *(tr++);
            out->SET.elem= malloc(out->SET.count* sizeof(*out->SET.elem));
            if(!out->SET.elem) {
                return 0;
            }
            for(pos i=0; i<out->SET.count; i++) {
                if(i>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    off  = *tr;
                    tr++;
                }
                off = bind_intconstant(&out->SET.elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    case NEGATE:
        tr = trace_begin + *tr;
        out->N_type= NEGATE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        out->NEGATE= malloc(sizeof(*out->NEGATE));
        if(!out->NEGATE) {
            return -1;
        }
        off = bind_intconstraint(out->NEGATE, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_constrainedint(constrainedint*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    off = bind_intp(&out->parser, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constraint= malloc(sizeof(*out->constraint));
        if(!out->constraint) return -1;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_intconstraint(&out->constraint[0], data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
    }
    else {
        tr = trace_begin + *tr;
        out->constraint= NULL;
    }*trace = tr;
    return off;
}
static pos bind_structparser(structparser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            if(i>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                off  = *tr;
                tr++;
            }
            fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
            switch(*(tr++)) {
            case CONSTANT:
                tr = trace_begin + *tr;
                out->elem[i].N_type= CONSTANT;
                off = bind_constparser(&out->elem[i].CONSTANT, data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
                break;
            case CTXT:
                tr = trace_begin + *tr;
                out->elem[i].N_type= CTXT;
                off = bind_contextidentifier(&out->elem[i].CTXT.name, data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
                out->elem[i].CTXT.parser= malloc(sizeof(*out->elem[i].CTXT.parser));
                if(!out->elem[i].CTXT.parser) {
                    return -1;
                }
                off = bind_parser(out->elem[i].CTXT.parser, data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
                break;
            case FIELD:
                tr = trace_begin + *tr;
                out->elem[i].N_type= FIELD;
                off = bind_varidentifier(&out->elem[i].FIELD.name, data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
                out->elem[i].FIELD.parser= malloc(sizeof(*out->elem[i].FIELD.parser));
                if(!out->elem[i].FIELD.parser) {
                    return -1;
                }
                off = bind_parser(out->elem[i].FIELD.parser, data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
                break;
            default:
                assert("BUG");
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    *trace = tr;
    return off;
}
static pos bind_wrapparser(wrapparser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constbefore= malloc(sizeof(*out->constbefore));
        if(!out->constbefore) return -1;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->constbefore[0].count=*(tr++);
            save = *(tr++);
            out->constbefore[0].elem= malloc(out->constbefore[0].count* sizeof(*out->constbefore[0].elem));
            if(!out->constbefore[0].elem) {
                return 0;
            }
            for(pos i=0; i<out->constbefore[0].count; i++) {
                if(i>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    off  = *tr;
                    tr++;
                }
                off = bind_constparser(&out->constbefore[0].elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
    }
    else {
        tr = trace_begin + *tr;
        out->constbefore= NULL;
    }
    out->parser= malloc(sizeof(*out->parser));
    if(!out->parser) {
        return -1;
    }
    off = bind_parser(out->parser, data,off,&tr,trace_begin);
    if(parser_fail(off)) {
        return -1;
    }
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constafter= malloc(sizeof(*out->constafter));
        if(!out->constafter) return -1;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->constafter[0].count=*(tr++);
            save = *(tr++);
            out->constafter[0].elem= malloc(out->constafter[0].count* sizeof(*out->constafter[0].elem));
            if(!out->constafter[0].elem) {
                return 0;
            }
            for(pos i=0; i<out->constafter[0].count; i++) {
                if(i>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    off  = *tr;
                    tr++;
                }
                off = bind_constparser(&out->constafter[0].elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
    }
    else {
        tr = trace_begin + *tr;
        out->constafter= NULL;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    *trace = tr;
    return off;
}
static pos bind_choiceparser(choiceparser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            off = bind_constidentifier(&out->elem[i].tag, data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
            fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
            off  = *tr;
            tr++;
            out->elem[i].parser= malloc(sizeof(*out->elem[i].parser));
            if(!out->elem[i].parser) {
                return -1;
            }
            off = bind_parser(out->elem[i].parser, data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    *trace = tr;
    return off;
}
static pos bind_arrayparser(arrayparser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case MANYONE:
        tr = trace_begin + *tr;
        out->N_type= MANYONE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        out->MANYONE= malloc(sizeof(*out->MANYONE));
        if(!out->MANYONE) {
            return -1;
        }
        off = bind_parser(out->MANYONE, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case MANY:
        tr = trace_begin + *tr;
        out->N_type= MANY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        out->MANY= malloc(sizeof(*out->MANY));
        if(!out->MANY) {
            return -1;
        }
        off = bind_parser(out->MANY, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case SEPBYONE:
        tr = trace_begin + *tr;
        out->N_type= SEPBYONE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_constparser(&out->SEPBYONE.separator, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        out->SEPBYONE.inner= malloc(sizeof(*out->SEPBYONE.inner));
        if(!out->SEPBYONE.inner) {
            return -1;
        }
        off = bind_parser(out->SEPBYONE.inner, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case SEPBY:
        tr = trace_begin + *tr;
        out->N_type= SEPBY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_constparser(&out->SEPBY.separator, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        out->SEPBY.inner= malloc(sizeof(*out->SEPBY.inner));
        if(!out->SEPBY.inner) {
            return -1;
        }
        off = bind_parser(out->SEPBY.inner, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_parserinner(parserinner*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case INT:
        tr = trace_begin + *tr;
        out->N_type= INT;
        off = bind_constrainedint(&out->INT, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case STRUCT:
        tr = trace_begin + *tr;
        out->N_type= STRUCT;
        off = bind_structparser(&out->STRUCT, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case WRAP:
        tr = trace_begin + *tr;
        out->N_type= WRAP;
        off = bind_wrapparser(&out->WRAP, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CHOICE:
        tr = trace_begin + *tr;
        out->N_type= CHOICE;
        off = bind_choiceparser(&out->CHOICE, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case ARRAY:
        tr = trace_begin + *tr;
        out->N_type= ARRAY;
        off = bind_arrayparser(&out->ARRAY, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case OPTIONAL:
        tr = trace_begin + *tr;
        out->N_type= OPTIONAL;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        out->OPTIONAL= malloc(sizeof(*out->OPTIONAL));
        if(!out->OPTIONAL) {
            return -1;
        }
        off = bind_parser(out->OPTIONAL, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case UNION:
        tr = trace_begin + *tr;
        out->N_type= UNION;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->UNION.count=*(tr++);
            save = *(tr++);
            out->UNION.elem= malloc(out->UNION.count* sizeof(*out->UNION.elem));
            if(!out->UNION.elem) {
                return 0;
            }
            for(pos i=0; i<out->UNION.count; i++) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                off  = *tr;
                tr++;
                out->UNION.elem[i]= malloc(sizeof(*out->UNION.elem[i]));
                if(!out->UNION.elem[i]) {
                    return -1;
                }
                off = bind_parser(out->UNION.elem[i], data,off,&tr,trace_begin);
                if(parser_fail(off)) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        break;
    case REF:
        tr = trace_begin + *tr;
        out->N_type= REF;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_varidentifier(&out->REF, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case NAME:
        tr = trace_begin + *tr;
        out->N_type= NAME;
        off = bind_varidentifier(&out->NAME, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_parser(parser*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case PAREN:
        tr = trace_begin + *tr;
        out->N_type= PAREN;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_parserinner(&out->PAREN, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        break;
    case PR:
        tr = trace_begin + *tr;
        out->N_type= PR;
        off = bind_parserinner(&out->PR, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_definition(definition*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case PARSER:
        tr = trace_begin + *tr;
        out->N_type= PARSER;
        off = bind_varidentifier(&out->PARSER.name, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_parser(&out->PARSER.definition, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    case CONST:
        tr = trace_begin + *tr;
        out->N_type= CONST;
        off = bind_constidentifier(&out->CONST.name, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        off  = *tr;
        tr++;
        off = bind_constparser(&out->CONST.definition, data,off,&tr,trace_begin);
        if(parser_fail(off)) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return off;
}
static pos bind_grammar(grammar*out,const char *data, pos off, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= malloc(out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i=0; i<out->count; i++) {
            off = bind_definition(&out->elem[i], data,off,&tr,trace_begin);
            if(parser_fail(off)) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    off  = *tr;
    tr++;
    *trace = tr;
    return off;
}
#include <hammer/hammer.h>
void gen_number(HBitWriter *out,number * val);
void gen_varidentifier(HBitWriter *out,varidentifier * val);
void gen_constidentifier(HBitWriter *out,constidentifier * val);
void gen_contextidentifier(HBitWriter *out,contextidentifier * val);
void gen_WHITE(HBitWriter* out);
void gen_SEPERATOR(HBitWriter* out);
void gen_intconstant(HBitWriter *out,intconstant * val);
void gen_intp(HBitWriter *out,intp * val);
void gen_constint(HBitWriter *out,constint * val);
void gen_arrayvalue(HBitWriter *out,arrayvalue * val);
void gen_constarray(HBitWriter *out,constarray * val);
void gen_constfields(HBitWriter *out,constfields * val);
void gen_constparser(HBitWriter *out,constparser * val);
void gen_intconstraint(HBitWriter *out,intconstraint * val);
void gen_constrainedint(HBitWriter *out,constrainedint * val);
void gen_structparser(HBitWriter *out,structparser * val);
void gen_wrapparser(HBitWriter *out,wrapparser * val);
void gen_choiceparser(HBitWriter *out,choiceparser * val);
void gen_arrayparser(HBitWriter *out,arrayparser * val);
void gen_parserinner(HBitWriter *out,parserinner * val);
void gen_parser(HBitWriter *out,parser * val);
void gen_definition(HBitWriter *out,definition * val);
void gen_grammar(HBitWriter *out,grammar * val);
void gen_number(HBitWriter *out,number * val) {
    for(int i0=0; i0<val->count; i0++) {
        h_bit_writer_put(out,val->elem[i0],8);
    }
}
void gen_varidentifier(HBitWriter *out,varidentifier * val) {
    gen_WHITE(out);
    for(int i1=0; i1<val->count; i1++) {
        h_bit_writer_put(out,val->elem[i1],8);
    }
}
void gen_constidentifier(HBitWriter *out,constidentifier * val) {
    gen_WHITE(out);
    for(int i2=0; i2<val->count; i2++) {
        h_bit_writer_put(out,val->elem[i2],8);
    }
}
void gen_contextidentifier(HBitWriter *out,contextidentifier * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'@',8);
    for(int i3=0; i3<val->count; i3++) {
        h_bit_writer_put(out,val->elem[i3],8);
    }
}
void gen_WHITE(HBitWriter* out) {
    h_bit_writer_put(out,' ',8);
}
void gen_SEPERATOR(HBitWriter* out) {
    h_bit_writer_put(out,' ',8);
    h_bit_writer_put(out,'\n',8);
}
void gen_intconstant(HBitWriter *out,intconstant * val) {
    gen_WHITE(out);
    switch(val->N_type) {
    case ASCII:
        h_bit_writer_put(out,'\'',8);
        switch(val->ASCII.N_type) {
        case ESCAPE:
            h_bit_writer_put(out,'\\',8);
            h_bit_writer_put(out,val->ASCII.ESCAPE,8);
            break;
        case DIRECT:
            h_bit_writer_put(out,val->ASCII.DIRECT,8);
            break;
        }
        h_bit_writer_put(out,'\'',8);
        break;
    case NUMBER:
        gen_number(out,&val->NUMBER);
        break;
    }
}
void gen_intp(HBitWriter *out,intp * val) {
    switch(val->N_type) {
    case UNSIGNED:
        gen_WHITE(out);
        h_bit_writer_put(out,'u',8);
        h_bit_writer_put(out,'i',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'t',8);
        for(int i4=0; i4<val->UNSIGNED.count; i4++) {
            h_bit_writer_put(out,val->UNSIGNED.elem[i4],8);
        }
        break;
    case SIGNED:
        gen_WHITE(out);
        h_bit_writer_put(out,'i',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'t',8);
        for(int i5=0; i5<val->SIGNED.count; i5++) {
            h_bit_writer_put(out,val->SIGNED.elem[i5],8);
        }
        break;
    }
}
void gen_constint(HBitWriter *out,constint * val) {
    gen_intp(out,&val->parser);
    gen_WHITE(out);
    h_bit_writer_put(out,'=',8);
    gen_intconstant(out,&val->value);
}
void gen_arrayvalue(HBitWriter *out,arrayvalue * val) {
    switch(val->N_type) {
    case STRING:
        gen_WHITE(out);
        h_bit_writer_put(out,'"',8);
        for(int i6=0; i6<val->STRING.count; i6++) {
            h_bit_writer_put(out,val->STRING.elem[i6],8);
        }
        h_bit_writer_put(out,'"',8);
        break;
    case VALUES:
        gen_WHITE(out);
        h_bit_writer_put(out,'[',8);
        for(int i7=0; i7<val->VALUES.count; i7++) {
            gen_intconstant(out,&val->VALUES.elem[i7]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,']',8);
        break;
    }
}
void gen_constarray(HBitWriter *out,constarray * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'m',8);
    h_bit_writer_put(out,'a',8);
    h_bit_writer_put(out,'n',8);
    h_bit_writer_put(out,'y',8);
    h_bit_writer_put(out,' ',8);
    gen_intp(out,&val->parser);
    gen_WHITE(out);
    h_bit_writer_put(out,'=',8);
    gen_arrayvalue(out,&val->value);
}
void gen_constfields(HBitWriter *out,constfields * val) {
    for(int i8=0; i8<val->count; i8++) {
        if(i8!= 0) {
            gen_SEPERATOR(out);
        }
        gen_constparser(out,&val->elem[i8]);
    }
}
void gen_constparser(HBitWriter *out,constparser * val) {
    switch(val->N_type) {
    case CARRAY:
        gen_constarray(out,&val->CARRAY);
        break;
    case CREPEAT:
        gen_WHITE(out);
        h_bit_writer_put(out,'m',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'y',8);
        h_bit_writer_put(out,' ',8);
        gen_constparser(out,val->CREPEAT);
        break;
    case CINT:
        gen_constint(out,&val->CINT);
        break;
    case CREF:
        gen_constidentifier(out,&val->CREF);
        break;
    case CSTRUCT:
        gen_WHITE(out);
        h_bit_writer_put(out,'{',8);
        gen_constfields(out,&val->CSTRUCT);
        gen_WHITE(out);
        h_bit_writer_put(out,'}',8);
        break;
    case CUNION:
        for(int i9=0; i9<val->CUNION.count; i9++) {
            gen_WHITE(out);
            h_bit_writer_put(out,'|',8);
            h_bit_writer_put(out,'|',8);
            gen_constparser(out,&val->CUNION.elem[i9]);
        }
        break;
    }
}
void gen_intconstraint(HBitWriter *out,intconstraint * val) {
    switch(val->N_type) {
    case RANGE:
        if(NULL!=val->RANGE.min) {
            gen_intconstant(out,&val->RANGE.min[0]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,'.',8);
        h_bit_writer_put(out,'.',8);
        if(NULL!=val->RANGE.max) {
            gen_intconstant(out,&val->RANGE.max[0]);
        }
        break;
    case SET:
        gen_WHITE(out);
        h_bit_writer_put(out,'[',8);
        for(int i10=0; i10<val->SET.count; i10++) {
            if(i10!= 0) {
                h_bit_writer_put(out,',',8);
            }
            gen_intconstant(out,&val->SET.elem[i10]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,']',8);
        break;
    case NEGATE:
        gen_WHITE(out);
        h_bit_writer_put(out,'!',8);
        gen_intconstraint(out,val->NEGATE);
        break;
    }
}
void gen_constrainedint(HBitWriter *out,constrainedint * val) {
    gen_intp(out,&val->parser);
    if(NULL!=val->constraint) {
        gen_WHITE(out);
        h_bit_writer_put(out,'|',8);
        gen_intconstraint(out,&val->constraint[0]);
    }
}
void gen_structparser(HBitWriter *out,structparser * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'{',8);
    for(int i11=0; i11<val->count; i11++) {
        if(i11!= 0) {
            gen_SEPERATOR(out);
        }
        switch(val->elem[i11].N_type) {
        case CONSTANT:
            gen_constparser(out,&val->elem[i11].CONSTANT);
            break;
        case CTXT:
            gen_contextidentifier(out,&val->elem[i11].CTXT.name);
            gen_parser(out,val->elem[i11].CTXT.parser);
            break;
        case FIELD:
            gen_varidentifier(out,&val->elem[i11].FIELD.name);
            gen_parser(out,val->elem[i11].FIELD.parser);
            break;
        }
    }
    gen_WHITE(out);
    h_bit_writer_put(out,'}',8);
}
void gen_wrapparser(HBitWriter *out,wrapparser * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'<',8);
    if(NULL!=val->constbefore) {
        for(int i12=0; i12<val->constbefore[0].count; i12++) {
            if(i12!= 0) {
                gen_SEPERATOR(out);
            }
            gen_constparser(out,&val->constbefore[0].elem[i12]);
        }
        gen_SEPERATOR(out);
    }
    gen_parser(out,val->parser);
    if(NULL!=val->constafter) {
        gen_SEPERATOR(out);
        for(int i13=0; i13<val->constafter[0].count; i13++) {
            if(i13!= 0) {
                gen_SEPERATOR(out);
            }
            gen_constparser(out,&val->constafter[0].elem[i13]);
        }
    }
    gen_WHITE(out);
    h_bit_writer_put(out,'>',8);
}
void gen_choiceparser(HBitWriter *out,choiceparser * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'c',8);
    h_bit_writer_put(out,'h',8);
    h_bit_writer_put(out,'o',8);
    h_bit_writer_put(out,'o',8);
    h_bit_writer_put(out,'s',8);
    h_bit_writer_put(out,'e',8);
    gen_WHITE(out);
    h_bit_writer_put(out,'{',8);
    for(int i14=0; i14<val->count; i14++) {
        gen_constidentifier(out,&val->elem[i14].tag);
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_parser(out,val->elem[i14].parser);
    }
    gen_WHITE(out);
    h_bit_writer_put(out,'}',8);
}
void gen_arrayparser(HBitWriter *out,arrayparser * val) {
    switch(val->N_type) {
    case MANYONE:
        gen_WHITE(out);
        h_bit_writer_put(out,'m',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'y',8);
        h_bit_writer_put(out,'1',8);
        gen_parser(out,val->MANYONE);
        break;
    case MANY:
        gen_WHITE(out);
        h_bit_writer_put(out,'m',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'y',8);
        gen_parser(out,val->MANY);
        break;
    case SEPBYONE:
        gen_WHITE(out);
        h_bit_writer_put(out,'s',8);
        h_bit_writer_put(out,'e',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'B',8);
        h_bit_writer_put(out,'y',8);
        h_bit_writer_put(out,'1',8);
        gen_constparser(out,&val->SEPBYONE.separator);
        gen_parser(out,val->SEPBYONE.inner);
        break;
    case SEPBY:
        gen_WHITE(out);
        h_bit_writer_put(out,'s',8);
        h_bit_writer_put(out,'e',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'B',8);
        h_bit_writer_put(out,'y',8);
        gen_constparser(out,&val->SEPBY.separator);
        gen_parser(out,val->SEPBY.inner);
        break;
    }
}
void gen_parserinner(HBitWriter *out,parserinner * val) {
    switch(val->N_type) {
    case INT:
        gen_constrainedint(out,&val->INT);
        break;
    case STRUCT:
        gen_structparser(out,&val->STRUCT);
        break;
    case WRAP:
        gen_wrapparser(out,&val->WRAP);
        break;
    case CHOICE:
        gen_choiceparser(out,&val->CHOICE);
        break;
    case ARRAY:
        gen_arrayparser(out,&val->ARRAY);
        break;
    case OPTIONAL:
        gen_WHITE(out);
        h_bit_writer_put(out,'o',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'t',8);
        h_bit_writer_put(out,'i',8);
        h_bit_writer_put(out,'o',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'l',8);
        h_bit_writer_put(out,' ',8);
        gen_parser(out,val->OPTIONAL);
        break;
    case UNION:
        for(int i15=0; i15<val->UNION.count; i15++) {
            gen_WHITE(out);
            h_bit_writer_put(out,'|',8);
            h_bit_writer_put(out,'|',8);
            gen_parser(out,val->UNION.elem[i15]);
        }
        break;
    case REF:
        gen_WHITE(out);
        h_bit_writer_put(out,'*',8);
        gen_varidentifier(out,&val->REF);
        break;
    case NAME:
        gen_varidentifier(out,&val->NAME);
        break;
    }
}
void gen_parser(HBitWriter *out,parser * val) {
    switch(val->N_type) {
    case PAREN:
        gen_WHITE(out);
        h_bit_writer_put(out,'(',8);
        gen_parserinner(out,&val->PAREN);
        gen_WHITE(out);
        h_bit_writer_put(out,')',8);
        break;
    case PR:
        gen_parserinner(out,&val->PR);
        break;
    }
}
void gen_definition(HBitWriter *out,definition * val) {
    switch(val->N_type) {
    case PARSER:
        gen_varidentifier(out,&val->PARSER.name);
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_parser(out,&val->PARSER.definition);
        break;
    case CONST:
        gen_constidentifier(out,&val->CONST.name);
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_WHITE(out);
        gen_constparser(out,&val->CONST.definition);
        break;
    }
}
void gen_grammar(HBitWriter *out,grammar * val) {
    for(int i16=0; i16<val->count; i16++) {
        gen_definition(out,&val->elem[i16]);
    }
    gen_WHITE(out);
}
#include <stdio.h>
extern HAllocator system_allocator;
#define TOKENPASTE(x,y) x ## y
#define CAT(x,y) TOKENPASTE(x,y)
int main() {
    FILE * dump = popen("od -t d4","w");
    uint8_t input[102400];
    size_t inputsize;
    n_trace test;
    pos p;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);
    fprintf(stderr,"\n\n\n");
    n_trace_init(&test,4096,4096);
    inputsize *= 8;
#if XYCONST > 0
    p = CAT(packrat_,XYZZY)(input,0,inputsize);
#else
    p = CAT(packrat_,XYZZY)(NULL,&test, input,0,inputsize);

#endif

    fprintf(stderr, "pos= %d\n", p);
    fwrite(test.trace,sizeof(pos),test.iter,dump);
    fclose(dump);

#if XYCONST == 0
    if(p==inputsize) {
        XYZZY object;
        HBitWriter *bw;
        pos *tr = test.trace;
        const char *buf;
        size_t siz;
        pos succ = CAT(bind_,XYZZY)(&object,input, 0, &tr, test.trace);
        bw= h_bit_writer_new(&system_allocator);
        if(succ<0)
            exit(-2);
        CAT(gen_,XYZZY)(bw,&object);
        buf = h_bit_writer_get_buffer(bw,&siz);
        printf("Output:\n");
        fwrite(buf,1,siz,stdout);
        printf("\nEnd of output;\n");
    }
#endif
    if(p!= inputsize )
        exit(-1);
    else
        exit(0);
}
