#include "new_grammar.nail.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef int32_t pos;
typedef struct {
    pos *trace;
    pos capacity,iter,grow;
} n_trace;
static uint64_t read_unsigned_bits(const NailStream *stream, unsigned count) {
    uint64_t retval = 0;
    unsigned int out_idx=count;
    size_t pos = stream->pos;
    const uint8_t *data = stream->data;
    //TODO: Implement little endian too
    //Count LSB to MSB
    while(count>0) {
        if((pos & 7) == 0 && (count &7) ==0) {
            out_idx-=8;
            retval|= data[pos >> 3] << out_idx;
            pos += 8;
            count-=8;
        }
        else {
            //This can use a lot of performance love
//TODO: implement other endianesses
            out_idx--;
            retval |= ((data[pos>>3] >> (7-(pos&7))) & 1) << out_idx;
            count--;
            pos++;
        }
    }
    return retval;
}
static int stream_check(const NailStream *stream, unsigned count) {
    return stream->size - count >= stream->pos;
}
static int stream_advance(NailStream *stream, unsigned count) {
    stream->pos += count;
    return 0;
}
static int stream_reposition(NailStream *stream, NailStreamPos p)
{
    assert(p <= stream->size);
    stream->pos = p;
    return 0;
}
static NailStreamPos   stream_getpos(NailStream *stream) {
    return stream->pos;
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
    out->trace = (pos *)malloc(size * sizeof(pos));
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
static void n_trace_release(n_trace *out) {
    free(out->trace);
    out->trace =NULL;
    out->capacity = 0;
    out->iter = 0;
    out->grow = 0;
}
static int n_trace_grow(n_trace *out, int space) {
    if(out->capacity - space>= out->iter) {
        return 0;
    }

    pos * new_ptr= (pos *)realloc(out->trace, out->capacity + out->grow);
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
static int n_tr_const(n_trace *trace,NailStream *stream) {
    if(n_trace_grow(trace,1))
        return -1;
    NailStreamPos newoff = stream_getpos(stream);
    fprintf(stderr,"%d = const %d \n",trace->iter, newoff);
    trace->trace[trace->iter++] = newoff;
    return 0;
}
#define n_tr_offset n_tr_const
typedef struct NailArenaPool {
    char *iter;
    char *end;
    struct NailArenaPool *next;
} NailArenaPool;

void *n_malloc(NailArena *arena, size_t size)
{
    void *retval;
    if(arena->current->end - arena->current->iter <= size) {
        size_t siz = arena->blocksize;
        if(size>siz)
            siz = size + sizeof(NailArenaPool);
        NailArenaPool *newpool  = (NailArenaPool *)malloc(siz);
        if(!newpool) return NULL;
        newpool->end = (char *)((char *)newpool + siz);
        newpool->iter = (char*)(newpool+1);
        newpool->next = arena->current;
        arena->current= newpool;
    }
    retval = (void *)arena->current->iter;
    arena->current->iter += size;
    memset(retval,0,size);
    return retval;
}

int NailArena_init(NailArena *arena, size_t blocksize) {
    if(blocksize< 2*sizeof(NailArena))
        blocksize = 2*sizeof(NailArena);
    arena->current = (NailArenaPool*)malloc(blocksize);
    if(!arena->current) return 0;
    arena->current->next = NULL;
    arena->current->iter = (char *)(arena->current + 1);
    arena->current->end = (char *) arena->current + blocksize;
    arena->blocksize = blocksize;
    return 1;
}
int NailArena_release(NailArena *arena) {
    NailArenaPool *p;
    while((p= arena->current) ) {
        arena->current = p->next;
        free(p);
    }
    arena->blocksize = 0;
    return 0;
}
//Returns the pointer where the taken choice is supposed to go.

#define parser_fail(i) __builtin_expect(i<0,0)



static pos peg_number(n_trace *trace,NailStream *str_current);
static pos peg_varidentifier(n_trace *trace,NailStream *str_current);
static pos peg_constidentifier(n_trace *trace,NailStream *str_current);
static pos peg_streamidentifier(n_trace *trace,NailStream *str_current);
static pos peg_dependencyidentifier(n_trace *trace,NailStream *str_current);
static pos peg_WHITE(NailStream *str_current);
static pos peg_SEPERATOR(NailStream *str_current);
static pos peg_intconstant(n_trace *trace,NailStream *str_current);
static pos peg_intp(n_trace *trace,NailStream *str_current);
static pos peg_constint(n_trace *trace,NailStream *str_current);
static pos peg_arrayvalue(n_trace *trace,NailStream *str_current);
static pos peg_constarray(n_trace *trace,NailStream *str_current);
static pos peg_constfields(n_trace *trace,NailStream *str_current);
static pos peg_constparser(n_trace *trace,NailStream *str_current);
static pos peg_constraintelem(n_trace *trace,NailStream *str_current);
static pos peg_intconstraint(n_trace *trace,NailStream *str_current);
static pos peg_constrainedint(n_trace *trace,NailStream *str_current);
static pos peg_transform(n_trace *trace,NailStream *str_current);
static pos peg_structparser(n_trace *trace,NailStream *str_current);
static pos peg_wrapparser(n_trace *trace,NailStream *str_current);
static pos peg_choiceparser(n_trace *trace,NailStream *str_current);
static pos peg_arrayparser(n_trace *trace,NailStream *str_current);
static pos peg_parameter(n_trace *trace,NailStream *str_current);
static pos peg_parameterlist(n_trace *trace,NailStream *str_current);
static pos peg_parameterdefinition(n_trace *trace,NailStream *str_current);
static pos peg_parameterdefinitionlist(n_trace *trace,NailStream *str_current);
static pos peg_parserinvocation(n_trace *trace,NailStream *str_current);
static pos peg_parserinner(n_trace *trace,NailStream *str_current);
static pos peg_parser(n_trace *trace,NailStream *str_current);
static pos peg_definition(n_trace *trace,NailStream *str_current);
static pos peg_grammar(n_trace *trace,NailStream *str_current);
static pos peg_number(n_trace *trace, NailStream *str_current) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_0:
        if(!stream_check(str_current,8)) {
            goto fail_repeat_0;
        }
        {
            uint64_t val = read_unsigned_bits(str_current,8);
            if((val>'9'||val<'0')) {
                goto fail_repeat_0;
            }
        }
        stream_advance(str_current,8);
        count++;
        goto succ_repeat_0;
fail_repeat_0:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_varidentifier(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_1:
        if(!stream_check(str_current,8)) {
            goto fail_repeat_1;
        }
        {
            uint64_t val = read_unsigned_bits(str_current,8);
            if((val>'z'||val<'a')) {
                goto fail_repeat_1;
            }
        }
        stream_advance(str_current,8);
        count++;
        goto succ_repeat_1;
fail_repeat_1:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_constidentifier(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_2:
        if(!stream_check(str_current,8)) {
            goto fail_repeat_2;
        }
        {
            uint64_t val = read_unsigned_bits(str_current,8);
            if((val>'Z'||val<'A')) {
                goto fail_repeat_2;
            }
        }
        stream_advance(str_current,8);
        count++;
        goto succ_repeat_2;
fail_repeat_2:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_streamidentifier(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '#') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_3:
        if(!stream_check(str_current,8)) {
            goto fail_repeat_3;
        }
        {
            uint64_t val = read_unsigned_bits(str_current,8);
            if((val>'z'||val<'a')) {
                goto fail_repeat_3;
            }
        }
        stream_advance(str_current,8);
        count++;
        goto succ_repeat_3;
fail_repeat_3:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_dependencyidentifier(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '@') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_4:
        if(!stream_check(str_current,8)) {
            goto fail_repeat_4;
        }
        {
            uint64_t val = read_unsigned_bits(str_current,8);
            if((val>'z'||val<'a')) {
                goto fail_repeat_4;
            }
        }
        stream_advance(str_current,8);
        count++;
        goto succ_repeat_4;
fail_repeat_4:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_WHITE(NailStream *str_current) {
constmany_0_repeat:
    {
        NailStreamPos back = stream_getpos(str_current);
        if(!stream_check(str_current,8)) {
            goto cunion_1_1;
        }
        if( read_unsigned_bits(str_current,8)!= ' ') {
            goto cunion_1_1;
        }
        stream_advance(str_current,8);
        goto cunion_1_succ;
cunion_1_1:
        stream_reposition(str_current,back);
        if(!stream_check(str_current,8)) {
            goto cunion_1_2;
        }
        if( read_unsigned_bits(str_current,8)!= '\n') {
            goto cunion_1_2;
        }
        stream_advance(str_current,8);
        goto cunion_1_succ;
cunion_1_2:
        stream_reposition(str_current,back);
        goto constmany_1_end;
cunion_1_succ:
        ;
    }
    goto constmany_0_repeat;
constmany_1_end:
    return 0;
fail:
    return -1;
}
static pos peg_SEPERATOR(NailStream *str_current) {
constmany_2_repeat:
    if(!stream_check(str_current,8)) {
        goto constmany_3_end;
    }
    if( read_unsigned_bits(str_current,8)!= ' ') {
        goto constmany_3_end;
    }
    stream_advance(str_current,8);
    goto constmany_2_repeat;
constmany_3_end:
constmany_4_repeat:
    {
        NailStreamPos back = stream_getpos(str_current);
        if(!stream_check(str_current,8)) {
            goto cunion_2_1;
        }
        if( read_unsigned_bits(str_current,8)!= '\n') {
            goto cunion_2_1;
        }
        stream_advance(str_current,8);
        goto cunion_2_succ;
cunion_2_1:
        stream_reposition(str_current,back);
        if(!stream_check(str_current,8)) {
            goto cunion_2_2;
        }
        if( read_unsigned_bits(str_current,8)!= ';') {
            goto cunion_2_2;
        }
        stream_advance(str_current,8);
        goto cunion_2_succ;
cunion_2_2:
        stream_reposition(str_current,back);
        goto constmany_5_end;
cunion_2_succ:
        ;
    }
    goto constmany_4_repeat;
constmany_5_end:
    return 0;
fail:
    return -1;
}
static pos peg_intconstant(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(!stream_check(str_current,8)) {
            goto choice_3_ASCII_out;
        }
        if( read_unsigned_bits(str_current,8)!= '\'') {
            goto choice_3_ASCII_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_3_ASCII_out;
        }
        {   NailStreamPos back = stream_getpos(str_current);
            pos choice_begin = n_tr_begin_choice(trace);
            pos choice;
            if(parser_fail(choice_begin)) {
                goto choice_3_ASCII_out;
            }
            choice = n_tr_memo_choice(trace);
            if(!stream_check(str_current,8)) {
                goto choice_4_ESCAPE_out;
            }
            if( read_unsigned_bits(str_current,8)!= '\\') {
                goto choice_4_ESCAPE_out;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto choice_4_ESCAPE_out;
            }
            if(!stream_check(str_current,8)) {
                goto choice_4_ESCAPE_out;
            }
            stream_advance(str_current,8);
            n_tr_pick_choice(trace,choice_begin,ESCAPE,choice);
            goto choice_4_succ;
choice_4_ESCAPE_out:
            stream_reposition(str_current, back);
            choice = n_tr_memo_choice(trace);
            if(!stream_check(str_current,8)) {
                goto choice_4_DIRECT_out;
            }
            stream_advance(str_current,8);
            n_tr_pick_choice(trace,choice_begin,DIRECT,choice);
            goto choice_4_succ;
choice_4_DIRECT_out:
            stream_reposition(str_current, back);
            goto choice_3_ASCII_out;
choice_4_succ:
            ;
        }
        if(!stream_check(str_current,8)) {
            goto choice_3_ASCII_out;
        }
        if( read_unsigned_bits(str_current,8)!= '\'') {
            goto choice_3_ASCII_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_3_ASCII_out;
        }
        n_tr_pick_choice(trace,choice_begin,ASCII,choice);
        goto choice_3_succ;
choice_3_ASCII_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(!stream_check(str_current,8)) {
            goto choice_3_HEX_out;
        }
        if( read_unsigned_bits(str_current,8)!= '0') {
            goto choice_3_HEX_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_3_HEX_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_3_HEX_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'x') {
            goto choice_3_HEX_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_3_HEX_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_5:
            {   NailStreamPos back = stream_getpos(str_current);
                pos choice_begin = n_tr_begin_choice(trace);
                pos choice;
                if(parser_fail(choice_begin)) {
                    goto fail_repeat_5;
                }
                choice = n_tr_memo_choice(trace);
                if(!stream_check(str_current,8)) {
                    goto choice_5_1_out;
                }
                {
                    uint64_t val = read_unsigned_bits(str_current,8);
                    if((val>'9'||val<'0')) {
                        goto choice_5_1_out;
                    }
                }
                stream_advance(str_current,8);
                n_tr_pick_choice(trace,choice_begin,1,choice);
                goto choice_5_succ;
choice_5_1_out:
                stream_reposition(str_current, back);
                choice = n_tr_memo_choice(trace);
                if(!stream_check(str_current,8)) {
                    goto choice_5_2_out;
                }
                {
                    uint64_t val = read_unsigned_bits(str_current,8);
                    if((val>'F'||val<'A')) {
                        goto choice_5_2_out;
                    }
                }
                stream_advance(str_current,8);
                n_tr_pick_choice(trace,choice_begin,2,choice);
                goto choice_5_succ;
choice_5_2_out:
                stream_reposition(str_current, back);
                goto fail_repeat_5;
choice_5_succ:
                ;
            }
            count++;
            goto succ_repeat_5;
fail_repeat_5:
            n_tr_write_many(trace,many,count);
        }
        n_tr_pick_choice(trace,choice_begin,HEX,choice);
        goto choice_3_succ;
choice_3_HEX_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_number(trace,str_current))) {
            goto choice_3_NUMBER_out;
        }
        n_tr_pick_choice(trace,choice_begin,NUMBER,choice);
        goto choice_3_succ;
choice_3_NUMBER_out:
        stream_reposition(str_current, back);
        goto fail;
choice_3_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_intp(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_6_UNSIGN_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_6_UNSIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'u') {
            goto choice_6_UNSIGN_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_6_UNSIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'i') {
            goto choice_6_UNSIGN_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_6_UNSIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_6_UNSIGN_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_6_UNSIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 't') {
            goto choice_6_UNSIGN_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_6_UNSIGN_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_6:
            if(!stream_check(str_current,8)) {
                goto fail_repeat_6;
            }
            {
                uint64_t val = read_unsigned_bits(str_current,8);
                if((val>'9'||val<'0')) {
                    goto fail_repeat_6;
                }
            }
            stream_advance(str_current,8);
            count++;
            goto succ_repeat_6;
fail_repeat_6:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_6_UNSIGN_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,UNSIGN,choice);
        goto choice_6_succ;
choice_6_UNSIGN_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_6_SIGN_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_6_SIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'i') {
            goto choice_6_SIGN_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_6_SIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_6_SIGN_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_6_SIGN_out;
        }
        if( read_unsigned_bits(str_current,8)!= 't') {
            goto choice_6_SIGN_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_6_SIGN_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_7:
            if(!stream_check(str_current,8)) {
                goto fail_repeat_7;
            }
            {
                uint64_t val = read_unsigned_bits(str_current,8);
                if((val>'9'||val<'0')) {
                    goto fail_repeat_7;
                }
            }
            stream_advance(str_current,8);
            count++;
            goto succ_repeat_7;
fail_repeat_7:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_6_SIGN_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,SIGN,choice);
        goto choice_6_succ;
choice_6_SIGN_out:
        stream_reposition(str_current, back);
        goto fail;
choice_6_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_constint(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_intp(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '=') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_intconstant(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_arrayvalue(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_7_STRING_out;
        }
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_7_STRING_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_7_STRING_out;
        }
        if( read_unsigned_bits(str_current,8)!= '"') {
            goto choice_7_STRING_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_7_STRING_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_8:
            if(!stream_check(str_current,8)) {
                goto fail_repeat_8;
            }
            {
                uint64_t val = read_unsigned_bits(str_current,8);
                if(!(val!='"')) {
                    goto fail_repeat_8;
                }
            }
            stream_advance(str_current,8);
            count++;
            goto succ_repeat_8;
fail_repeat_8:
            n_tr_write_many(trace,many,count);
        }
        if(!stream_check(str_current,8)) {
            goto choice_7_STRING_out;
        }
        if( read_unsigned_bits(str_current,8)!= '"') {
            goto choice_7_STRING_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_7_STRING_out;
        }
        n_tr_pick_choice(trace,choice_begin,STRING,choice);
        goto choice_7_succ;
choice_7_STRING_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_7_VALUES_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_7_VALUES_out;
        }
        if( read_unsigned_bits(str_current,8)!= '[') {
            goto choice_7_VALUES_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_7_VALUES_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_9:
            if(parser_fail(peg_intconstant(trace,str_current))) {
                goto fail_repeat_9;
            }
            count++;
            goto succ_repeat_9;
fail_repeat_9:
            n_tr_write_many(trace,many,count);
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_7_VALUES_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_7_VALUES_out;
        }
        if( read_unsigned_bits(str_current,8)!= ']') {
            goto choice_7_VALUES_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_7_VALUES_out;
        }
        n_tr_pick_choice(trace,choice_begin,VALUES,choice);
        goto choice_7_succ;
choice_7_VALUES_out:
        stream_reposition(str_current, back);
        goto fail;
choice_7_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_constarray(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'm') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'a') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'n') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'y') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= ' ') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_intp(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '=') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_arrayvalue(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_constfields(n_trace *trace, NailStream *str_current) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_10:
        if(count>0) {
            if(parser_fail(peg_SEPERATOR(str_current))) {
                goto fail_repeat_10;
            }
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_10;
            }
        }
        if(parser_fail(peg_constparser(trace,str_current))) {
            goto fail_repeat_10;
        }
        count++;
        goto succ_repeat_10;
fail_repeat_10:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
static pos peg_constparser(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constarray(trace,str_current))) {
            goto choice_8_CARRAY_out;
        }
        n_tr_pick_choice(trace,choice_begin,CARRAY,choice);
        goto choice_8_succ;
choice_8_CARRAY_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_8_CREPEAT_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_8_CREPEAT_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'm') {
            goto choice_8_CREPEAT_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_8_CREPEAT_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'a') {
            goto choice_8_CREPEAT_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_8_CREPEAT_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_8_CREPEAT_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_8_CREPEAT_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_8_CREPEAT_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_8_CREPEAT_out;
        }
        if( read_unsigned_bits(str_current,8)!= ' ') {
            goto choice_8_CREPEAT_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_8_CREPEAT_out;
        }
        if(parser_fail(peg_constparser(trace,str_current))) {
            goto choice_8_CREPEAT_out;
        }
        n_tr_pick_choice(trace,choice_begin,CREPEAT,choice);
        goto choice_8_succ;
choice_8_CREPEAT_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constint(trace,str_current))) {
            goto choice_8_CINT_out;
        }
        n_tr_pick_choice(trace,choice_begin,CINT,choice);
        goto choice_8_succ;
choice_8_CINT_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constidentifier(trace,str_current))) {
            goto choice_8_CREF_out;
        }
        n_tr_pick_choice(trace,choice_begin,CREF,choice);
        goto choice_8_succ;
choice_8_CREF_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_8_CSTRUCT_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_8_CSTRUCT_out;
        }
        if( read_unsigned_bits(str_current,8)!= '{') {
            goto choice_8_CSTRUCT_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_8_CSTRUCT_out;
        }
        if(parser_fail(peg_constfields(trace,str_current))) {
            goto choice_8_CSTRUCT_out;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_8_CSTRUCT_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_8_CSTRUCT_out;
        }
        if( read_unsigned_bits(str_current,8)!= '}') {
            goto choice_8_CSTRUCT_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_8_CSTRUCT_out;
        }
        n_tr_pick_choice(trace,choice_begin,CSTRUCT,choice);
        goto choice_8_succ;
choice_8_CSTRUCT_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_11:
            if(parser_fail(peg_WHITE(str_current))) {
                goto fail_repeat_11;
            }
            if(!stream_check(str_current,8)) {
                goto fail_repeat_11;
            }
            if( read_unsigned_bits(str_current,8)!= '|') {
                goto fail_repeat_11;
            }
            stream_advance(str_current,8);
            if(!stream_check(str_current,8)) {
                goto fail_repeat_11;
            }
            if( read_unsigned_bits(str_current,8)!= '|') {
                goto fail_repeat_11;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_11;
            }
            if(parser_fail(peg_constparser(trace,str_current))) {
                goto fail_repeat_11;
            }
            count++;
            goto succ_repeat_11;
fail_repeat_11:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_8_CUNION_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,CUNION,choice);
        goto choice_8_succ;
choice_8_CUNION_out:
        stream_reposition(str_current, back);
        goto fail;
choice_8_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_constraintelem(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_optional(trace);
            if(parser_fail(peg_intconstant(trace,str_current))) {
                goto fail_optional_12;
            }
            n_tr_optional_succeed(trace,many);
            goto succ_optional_12;
fail_optional_12:
            n_tr_optional_fail(trace,many);
succ_optional_12:
            ;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_9_RANGE_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_9_RANGE_out;
        }
        if( read_unsigned_bits(str_current,8)!= '.') {
            goto choice_9_RANGE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_9_RANGE_out;
        }
        if( read_unsigned_bits(str_current,8)!= '.') {
            goto choice_9_RANGE_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_9_RANGE_out;
        }
        {
            pos many = n_tr_memo_optional(trace);
            if(parser_fail(peg_intconstant(trace,str_current))) {
                goto fail_optional_13;
            }
            n_tr_optional_succeed(trace,many);
            goto succ_optional_13;
fail_optional_13:
            n_tr_optional_fail(trace,many);
succ_optional_13:
            ;
        }
        n_tr_pick_choice(trace,choice_begin,RANGE,choice);
        goto choice_9_succ;
choice_9_RANGE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_intconstant(trace,str_current))) {
            goto choice_9_VALUE_out;
        }
        n_tr_pick_choice(trace,choice_begin,VALUE,choice);
        goto choice_9_succ;
choice_9_VALUE_out:
        stream_reposition(str_current, back);
        goto fail;
choice_9_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_intconstraint(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constraintelem(trace,str_current))) {
            goto choice_10_SINGLE_out;
        }
        n_tr_pick_choice(trace,choice_begin,SINGLE,choice);
        goto choice_10_succ;
choice_10_SINGLE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_10_SET_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_10_SET_out;
        }
        if( read_unsigned_bits(str_current,8)!= '[') {
            goto choice_10_SET_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_10_SET_out;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_14:
            if(count>0) {
                if(!stream_check(str_current,8)) {
                    goto fail_repeat_14;
                }
                if( read_unsigned_bits(str_current,8)!= ',') {
                    goto fail_repeat_14;
                }
                stream_advance(str_current,8);
                if(parser_fail(n_tr_const(trace,str_current))) {
                    goto fail_repeat_14;
                }
            }
            if(parser_fail(peg_constraintelem(trace,str_current))) {
                goto fail_repeat_14;
            }
            count++;
            goto succ_repeat_14;
fail_repeat_14:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_10_SET_out;
            }
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_10_SET_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_10_SET_out;
        }
        if( read_unsigned_bits(str_current,8)!= ']') {
            goto choice_10_SET_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_10_SET_out;
        }
        n_tr_pick_choice(trace,choice_begin,SET,choice);
        goto choice_10_succ;
choice_10_SET_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_10_NEGATE_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_10_NEGATE_out;
        }
        if( read_unsigned_bits(str_current,8)!= '!') {
            goto choice_10_NEGATE_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_10_NEGATE_out;
        }
        if(parser_fail(peg_intconstraint(trace,str_current))) {
            goto choice_10_NEGATE_out;
        }
        n_tr_pick_choice(trace,choice_begin,NEGATE,choice);
        goto choice_10_succ;
choice_10_NEGATE_out:
        stream_reposition(str_current, back);
        goto fail;
choice_10_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_constrainedint(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_intp(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_optional(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto fail_optional_15;
        }
        if(!stream_check(str_current,8)) {
            goto fail_optional_15;
        }
        if( read_unsigned_bits(str_current,8)!= '|') {
            goto fail_optional_15;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto fail_optional_15;
        }
        if(parser_fail(peg_intconstraint(trace,str_current))) {
            goto fail_optional_15;
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_15;
fail_optional_15:
        n_tr_optional_fail(trace,many);
succ_optional_15:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_transform(n_trace *trace, NailStream *str_current) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_16:
        if(count>0) {
            if(parser_fail(peg_WHITE(str_current))) {
                goto fail_repeat_16;
            }
            if(!stream_check(str_current,8)) {
                goto fail_repeat_16;
            }
            if( read_unsigned_bits(str_current,8)!= ',') {
                goto fail_repeat_16;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_16;
            }
        }
        if(parser_fail(peg_streamidentifier(trace,str_current))) {
            goto fail_repeat_16;
        }
        count++;
        goto succ_repeat_16;
fail_repeat_16:
        n_tr_write_many(trace,many,count);
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 't') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'r') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'a') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'n') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 's') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'f') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'o') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'r') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'm') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_varidentifier(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_17:
        if(parser_fail(peg_parameter(trace,str_current))) {
            goto fail_repeat_17;
        }
        count++;
        goto succ_repeat_17;
fail_repeat_17:
        n_tr_write_many(trace,many,count);
    }
    return 0;
fail:
    return -1;
}
static pos peg_structparser(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '{') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_18:
        if(count>0) {
            if(parser_fail(peg_SEPERATOR(str_current))) {
                goto fail_repeat_18;
            }
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_18;
            }
        }
        {   NailStreamPos back = stream_getpos(str_current);
            pos choice_begin = n_tr_begin_choice(trace);
            pos choice;
            if(parser_fail(choice_begin)) {
                goto fail_repeat_18;
            }
            choice = n_tr_memo_choice(trace);
            if(parser_fail(peg_constparser(trace,str_current))) {
                goto choice_11_CONSTANT_out;
            }
            n_tr_pick_choice(trace,choice_begin,CONSTANT,choice);
            goto choice_11_succ;
choice_11_CONSTANT_out:
            stream_reposition(str_current, back);
            choice = n_tr_memo_choice(trace);
            if(parser_fail(peg_dependencyidentifier(trace,str_current))) {
                goto choice_11_DEPENDENCY_out;
            }
            if(parser_fail(peg_parser(trace,str_current))) {
                goto choice_11_DEPENDENCY_out;
            }
            n_tr_pick_choice(trace,choice_begin,DEPENDENCY,choice);
            goto choice_11_succ;
choice_11_DEPENDENCY_out:
            stream_reposition(str_current, back);
            choice = n_tr_memo_choice(trace);
            if(parser_fail(peg_varidentifier(trace,str_current))) {
                goto choice_11_FIELD_out;
            }
            if(parser_fail(peg_parser(trace,str_current))) {
                goto choice_11_FIELD_out;
            }
            n_tr_pick_choice(trace,choice_begin,FIELD,choice);
            goto choice_11_succ;
choice_11_FIELD_out:
            stream_reposition(str_current, back);
            choice = n_tr_memo_choice(trace);
            if(parser_fail(peg_transform(trace,str_current))) {
                goto choice_11_TRANSFORM_out;
            }
            n_tr_pick_choice(trace,choice_begin,TRANSFORM,choice);
            goto choice_11_succ;
choice_11_TRANSFORM_out:
            stream_reposition(str_current, back);
            goto fail_repeat_18;
choice_11_succ:
            ;
        }
        count++;
        goto succ_repeat_18;
fail_repeat_18:
        n_tr_write_many(trace,many,count);
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '}') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_wrapparser(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '<') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_optional(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_20:
            if(count>0) {
                if(parser_fail(peg_SEPERATOR(str_current))) {
                    goto fail_repeat_20;
                }
                if(parser_fail(n_tr_const(trace,str_current))) {
                    goto fail_repeat_20;
                }
            }
            if(parser_fail(peg_constparser(trace,str_current))) {
                goto fail_repeat_20;
            }
            count++;
            goto succ_repeat_20;
fail_repeat_20:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto fail_optional_19;
            }
        }
        if(parser_fail(peg_SEPERATOR(str_current))) {
            goto fail_optional_19;
        }
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto fail_optional_19;
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_19;
fail_optional_19:
        n_tr_optional_fail(trace,many);
succ_optional_19:
        ;
    }
    if(parser_fail(peg_parser(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_optional(trace);
        if(parser_fail(peg_SEPERATOR(str_current))) {
            goto fail_optional_21;
        }
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto fail_optional_21;
        }
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_22:
            if(count>0) {
                if(parser_fail(peg_SEPERATOR(str_current))) {
                    goto fail_repeat_22;
                }
                if(parser_fail(n_tr_const(trace,str_current))) {
                    goto fail_repeat_22;
                }
            }
            if(parser_fail(peg_constparser(trace,str_current))) {
                goto fail_repeat_22;
            }
            count++;
            goto succ_repeat_22;
fail_repeat_22:
            n_tr_write_many(trace,many,count);
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_21;
fail_optional_21:
        n_tr_optional_fail(trace,many);
succ_optional_21:
        ;
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '>') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_choiceparser(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'c') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'h') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'o') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'o') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 's') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 'e') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '{') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_23:
        if(parser_fail(peg_constidentifier(trace,str_current))) {
            goto fail_repeat_23;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto fail_repeat_23;
        }
        if(!stream_check(str_current,8)) {
            goto fail_repeat_23;
        }
        if( read_unsigned_bits(str_current,8)!= '=') {
            goto fail_repeat_23;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto fail_repeat_23;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto fail_repeat_23;
        }
        count++;
        goto succ_repeat_23;
fail_repeat_23:
        n_tr_write_many(trace,many,count);
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '}') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_arrayparser(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_12_MANYONE_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_12_MANYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'm') {
            goto choice_12_MANYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'a') {
            goto choice_12_MANYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_12_MANYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_12_MANYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= '1') {
            goto choice_12_MANYONE_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_12_MANYONE_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_12_MANYONE_out;
        }
        n_tr_pick_choice(trace,choice_begin,MANYONE,choice);
        goto choice_12_succ;
choice_12_MANYONE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_12_MANY_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_12_MANY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'm') {
            goto choice_12_MANY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'a') {
            goto choice_12_MANY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_12_MANY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_MANY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_12_MANY_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_12_MANY_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_12_MANY_out;
        }
        n_tr_pick_choice(trace,choice_begin,MANY,choice);
        goto choice_12_succ;
choice_12_MANY_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_12_SEPBYONE_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 's') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'e') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'p') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'B') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBYONE_out;
        }
        if( read_unsigned_bits(str_current,8)!= '1') {
            goto choice_12_SEPBYONE_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_12_SEPBYONE_out;
        }
        if(parser_fail(peg_constparser(trace,str_current))) {
            goto choice_12_SEPBYONE_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_12_SEPBYONE_out;
        }
        n_tr_pick_choice(trace,choice_begin,SEPBYONE,choice);
        goto choice_12_succ;
choice_12_SEPBYONE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_12_SEPBY_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 's') {
            goto choice_12_SEPBY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'e') {
            goto choice_12_SEPBY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'p') {
            goto choice_12_SEPBY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'B') {
            goto choice_12_SEPBY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_12_SEPBY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_12_SEPBY_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_12_SEPBY_out;
        }
        if(parser_fail(peg_constparser(trace,str_current))) {
            goto choice_12_SEPBY_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_12_SEPBY_out;
        }
        n_tr_pick_choice(trace,choice_begin,SEPBY,choice);
        goto choice_12_succ;
choice_12_SEPBY_out:
        stream_reposition(str_current, back);
        goto fail;
choice_12_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parameter(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_dependencyidentifier(trace,str_current))) {
            goto choice_13_PDEPENDENCY_out;
        }
        n_tr_pick_choice(trace,choice_begin,PDEPENDENCY,choice);
        goto choice_13_succ;
choice_13_PDEPENDENCY_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_streamidentifier(trace,str_current))) {
            goto choice_13_PSTREAM_out;
        }
        n_tr_pick_choice(trace,choice_begin,PSTREAM,choice);
        goto choice_13_succ;
choice_13_PSTREAM_out:
        stream_reposition(str_current, back);
        goto fail;
choice_13_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parameterlist(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '(') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_24:
        if(count>0) {
            if(parser_fail(peg_WHITE(str_current))) {
                goto fail_repeat_24;
            }
            if(!stream_check(str_current,8)) {
                goto fail_repeat_24;
            }
            if( read_unsigned_bits(str_current,8)!= ',') {
                goto fail_repeat_24;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_24;
            }
        }
        if(parser_fail(peg_parameter(trace,str_current))) {
            goto fail_repeat_24;
        }
        count++;
        goto succ_repeat_24;
fail_repeat_24:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= ')') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parameterdefinition(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_streamidentifier(trace,str_current))) {
            goto choice_14_DSTREAM_out;
        }
        n_tr_pick_choice(trace,choice_begin,DSTREAM,choice);
        goto choice_14_succ;
choice_14_DSTREAM_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_dependencyidentifier(trace,str_current))) {
            goto choice_14_DDEPENDENCY_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_14_DDEPENDENCY_out;
        }
        n_tr_pick_choice(trace,choice_begin,DDEPENDENCY,choice);
        goto choice_14_succ;
choice_14_DDEPENDENCY_out:
        stream_reposition(str_current, back);
        goto fail;
choice_14_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parameterdefinitionlist(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= '(') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_25:
        if(count>0) {
            if(parser_fail(peg_WHITE(str_current))) {
                goto fail_repeat_25;
            }
            if(!stream_check(str_current,8)) {
                goto fail_repeat_25;
            }
            if( read_unsigned_bits(str_current,8)!= ',') {
                goto fail_repeat_25;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_25;
            }
        }
        if(parser_fail(peg_parameterdefinition(trace,str_current))) {
            goto fail_repeat_25;
        }
        count++;
        goto succ_repeat_25;
fail_repeat_25:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(!stream_check(str_current,8)) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= ')') {
        goto fail;
    }
    stream_advance(str_current,8);
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parserinvocation(n_trace *trace, NailStream *str_current) {
    pos i;
    if(parser_fail(peg_varidentifier(trace,str_current))) {
        goto fail;
    }
    {
        pos many = n_tr_memo_optional(trace);
        if(parser_fail(peg_parameterlist(trace,str_current))) {
            goto fail_optional_26;
        }
        n_tr_optional_succeed(trace,many);
        goto succ_optional_26;
fail_optional_26:
        n_tr_optional_fail(trace,many);
succ_optional_26:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parserinner(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constrainedint(trace,str_current))) {
            goto choice_15_INTEGER_out;
        }
        n_tr_pick_choice(trace,choice_begin,INTEGER,choice);
        goto choice_15_succ;
choice_15_INTEGER_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_structparser(trace,str_current))) {
            goto choice_15_STRUCTURE_out;
        }
        n_tr_pick_choice(trace,choice_begin,STRUCTURE,choice);
        goto choice_15_succ;
choice_15_STRUCTURE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_wrapparser(trace,str_current))) {
            goto choice_15_WRAP_out;
        }
        n_tr_pick_choice(trace,choice_begin,WRAP,choice);
        goto choice_15_succ;
choice_15_WRAP_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_choiceparser(trace,str_current))) {
            goto choice_15_CHOICE_out;
        }
        n_tr_pick_choice(trace,choice_begin,CHOICE,choice);
        goto choice_15_succ;
choice_15_CHOICE_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_arrayparser(trace,str_current))) {
            goto choice_15_ARRAY_out;
        }
        n_tr_pick_choice(trace,choice_begin,ARRAY,choice);
        goto choice_15_succ;
choice_15_ARRAY_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_FIXEDARRAY_out;
        }
        if( read_unsigned_bits(str_current,8)!= '[') {
            goto choice_15_FIXEDARRAY_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        if(parser_fail(peg_intconstant(trace,str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_FIXEDARRAY_out;
        }
        if( read_unsigned_bits(str_current,8)!= ']') {
            goto choice_15_FIXEDARRAY_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_15_FIXEDARRAY_out;
        }
        n_tr_pick_choice(trace,choice_begin,FIXEDARRAY,choice);
        goto choice_15_succ;
choice_15_FIXEDARRAY_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_LENGTH_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_LENGTH_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_15_LENGTH_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_LENGTH_out;
        }
        if( read_unsigned_bits(str_current,8)!= '_') {
            goto choice_15_LENGTH_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_LENGTH_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'o') {
            goto choice_15_LENGTH_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_LENGTH_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'f') {
            goto choice_15_LENGTH_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_LENGTH_out;
        }
        if(parser_fail(peg_dependencyidentifier(trace,str_current))) {
            goto choice_15_LENGTH_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_15_LENGTH_out;
        }
        n_tr_pick_choice(trace,choice_begin,LENGTH,choice);
        goto choice_15_succ;
choice_15_LENGTH_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_OPTIONAL_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'o') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'p') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 't') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'i') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'o') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'n') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'a') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'l') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_OPTIONAL_out;
        }
        if( read_unsigned_bits(str_current,8)!= ' ') {
            goto choice_15_OPTIONAL_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_OPTIONAL_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_15_OPTIONAL_out;
        }
        n_tr_pick_choice(trace,choice_begin,OPTIONAL,choice);
        goto choice_15_succ;
choice_15_OPTIONAL_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        {
            pos many = n_tr_memo_many(trace);
            pos count = 0;
succ_repeat_27:
            if(parser_fail(peg_WHITE(str_current))) {
                goto fail_repeat_27;
            }
            if(!stream_check(str_current,8)) {
                goto fail_repeat_27;
            }
            if( read_unsigned_bits(str_current,8)!= '|') {
                goto fail_repeat_27;
            }
            stream_advance(str_current,8);
            if(!stream_check(str_current,8)) {
                goto fail_repeat_27;
            }
            if( read_unsigned_bits(str_current,8)!= '|') {
                goto fail_repeat_27;
            }
            stream_advance(str_current,8);
            if(parser_fail(n_tr_const(trace,str_current))) {
                goto fail_repeat_27;
            }
            if(parser_fail(peg_parser(trace,str_current))) {
                goto fail_repeat_27;
            }
            count++;
            goto succ_repeat_27;
fail_repeat_27:
            n_tr_write_many(trace,many,count);
            if(count < 1) {
                goto choice_15_NUNION_out;
            }
        }
        n_tr_pick_choice(trace,choice_begin,NUNION,choice);
        goto choice_15_succ;
choice_15_NUNION_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_REF_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_REF_out;
        }
        if( read_unsigned_bits(str_current,8)!= '*') {
            goto choice_15_REF_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_REF_out;
        }
        if(parser_fail(peg_parserinvocation(trace,str_current))) {
            goto choice_15_REF_out;
        }
        n_tr_pick_choice(trace,choice_begin,REF,choice);
        goto choice_15_succ;
choice_15_REF_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_parserinvocation(trace,str_current))) {
            goto choice_15_NAME_out;
        }
        n_tr_pick_choice(trace,choice_begin,NAME,choice);
        goto choice_15_succ;
choice_15_NAME_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_15_APPLY_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_15_APPLY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'a') {
            goto choice_15_APPLY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_APPLY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'p') {
            goto choice_15_APPLY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_APPLY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'p') {
            goto choice_15_APPLY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_APPLY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'l') {
            goto choice_15_APPLY_out;
        }
        stream_advance(str_current,8);
        if(!stream_check(str_current,8)) {
            goto choice_15_APPLY_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'y') {
            goto choice_15_APPLY_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_15_APPLY_out;
        }
        if(parser_fail(peg_streamidentifier(trace,str_current))) {
            goto choice_15_APPLY_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_15_APPLY_out;
        }
        n_tr_pick_choice(trace,choice_begin,APPLY,choice);
        goto choice_15_succ;
choice_15_APPLY_out:
        stream_reposition(str_current, back);
        goto fail;
choice_15_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_parser(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_16_PAREN_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_16_PAREN_out;
        }
        if( read_unsigned_bits(str_current,8)!= '(') {
            goto choice_16_PAREN_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_16_PAREN_out;
        }
        if(parser_fail(peg_parserinner(trace,str_current))) {
            goto choice_16_PAREN_out;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_16_PAREN_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_16_PAREN_out;
        }
        if( read_unsigned_bits(str_current,8)!= ')') {
            goto choice_16_PAREN_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_16_PAREN_out;
        }
        n_tr_pick_choice(trace,choice_begin,PAREN,choice);
        goto choice_16_succ;
choice_16_PAREN_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_parserinner(trace,str_current))) {
            goto choice_16_PR_out;
        }
        n_tr_pick_choice(trace,choice_begin,PR,choice);
        goto choice_16_succ;
choice_16_PR_out:
        stream_reposition(str_current, back);
        goto fail;
choice_16_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_definition(n_trace *trace, NailStream *str_current) {
    pos i;
    {   NailStreamPos back = stream_getpos(str_current);
        pos choice_begin = n_tr_begin_choice(trace);
        pos choice;
        if(parser_fail(choice_begin)) {
            goto fail;
        }
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_varidentifier(trace,str_current))) {
            goto choice_17_PARSER_out;
        }
        {
            pos many = n_tr_memo_optional(trace);
            if(parser_fail(peg_parameterdefinitionlist(trace,str_current))) {
                goto fail_optional_28;
            }
            n_tr_optional_succeed(trace,many);
            goto succ_optional_28;
fail_optional_28:
            n_tr_optional_fail(trace,many);
succ_optional_28:
            ;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_17_PARSER_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_17_PARSER_out;
        }
        if( read_unsigned_bits(str_current,8)!= '=') {
            goto choice_17_PARSER_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_17_PARSER_out;
        }
        if(parser_fail(peg_parser(trace,str_current))) {
            goto choice_17_PARSER_out;
        }
        n_tr_pick_choice(trace,choice_begin,PARSER,choice);
        goto choice_17_succ;
choice_17_PARSER_out:
        stream_reposition(str_current, back);
        choice = n_tr_memo_choice(trace);
        if(parser_fail(peg_constidentifier(trace,str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(!stream_check(str_current,8)) {
            goto choice_17_CONSTANTDEF_out;
        }
        if( read_unsigned_bits(str_current,8)!= '=') {
            goto choice_17_CONSTANTDEF_out;
        }
        stream_advance(str_current,8);
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(parser_fail(peg_WHITE(str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(parser_fail(n_tr_const(trace,str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        if(parser_fail(peg_constparser(trace,str_current))) {
            goto choice_17_CONSTANTDEF_out;
        }
        n_tr_pick_choice(trace,choice_begin,CONSTANTDEF,choice);
        goto choice_17_succ;
choice_17_CONSTANTDEF_out:
        stream_reposition(str_current, back);
        goto fail;
choice_17_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
static pos peg_grammar(n_trace *trace, NailStream *str_current) {
    pos i;
    {
        pos many = n_tr_memo_many(trace);
        pos count = 0;
succ_repeat_29:
        if(parser_fail(peg_definition(trace,str_current))) {
            goto fail_repeat_29;
        }
        count++;
        goto succ_repeat_29;
fail_repeat_29:
        n_tr_write_many(trace,many,count);
        if(count < 1) {
            goto fail;
        }
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(n_tr_const(trace,str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}

static pos bind_number(NailArena *arena,number*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_varidentifier(NailArena *arena,varidentifier*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constidentifier(NailArena *arena,constidentifier*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_streamidentifier(NailArena *arena,streamidentifier*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_dependencyidentifier(NailArena *arena,dependencyidentifier*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_intconstant(NailArena *arena,intconstant*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_intp(NailArena *arena,intp*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constint(NailArena *arena,constint*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_arrayvalue(NailArena *arena,arrayvalue*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constarray(NailArena *arena,constarray*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constfields(NailArena *arena,constfields*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constparser(NailArena *arena,constparser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constraintelem(NailArena *arena,constraintelem*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_intconstraint(NailArena *arena,intconstraint*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_constrainedint(NailArena *arena,constrainedint*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_transform(NailArena *arena,transform*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_structparser(NailArena *arena,structparser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_wrapparser(NailArena *arena,wrapparser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_choiceparser(NailArena *arena,choiceparser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_arrayparser(NailArena *arena,arrayparser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parameter(NailArena *arena,parameter*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parameterlist(NailArena *arena,parameterlist*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parameterdefinition(NailArena *arena,parameterdefinition*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parameterdefinitionlist(NailArena *arena,parameterdefinitionlist*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parserinvocation(NailArena *arena,parserinvocation*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parserinner(NailArena *arena,parserinner*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_parser(NailArena *arena,parser*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_definition(NailArena *arena,definition*out,NailStream *stream, pos **trace,  pos * trace_begin);
static pos bind_grammar(NailArena *arena,grammar*out,NailStream *stream, pos **trace,  pos * trace_begin);
static int bind_number(NailArena *arena,number*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i1=0; i1<out->count; i1++) {
            out->elem[i1]=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
number*parse_number(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    number* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_number(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_number(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_varidentifier(NailArena *arena,varidentifier*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i2=0; i2<out->count; i2++) {
            out->elem[i2]=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
varidentifier*parse_varidentifier(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    varidentifier* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_varidentifier(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_varidentifier(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constidentifier(NailArena *arena,constidentifier*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i3=0; i3<out->count; i3++) {
            out->elem[i3]=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
constidentifier*parse_constidentifier(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constidentifier* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constidentifier(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constidentifier(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_streamidentifier(NailArena *arena,streamidentifier*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i4=0; i4<out->count; i4++) {
            out->elem[i4]=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
streamidentifier*parse_streamidentifier(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    streamidentifier* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_streamidentifier(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_streamidentifier(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_dependencyidentifier(NailArena *arena,dependencyidentifier*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i5=0; i5<out->count; i5++) {
            out->elem[i5]=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
dependencyidentifier*parse_dependencyidentifier(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    dependencyidentifier* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_dependencyidentifier(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_dependencyidentifier(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}


static int bind_intconstant(NailArena *arena,intconstant*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case ASCII:
        tr = trace_begin + *tr;
        out->N_type= ASCII;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
        switch(*(tr++)) {
        case ESCAPE:
            tr = trace_begin + *tr;
            out->ascii.N_type= ESCAPE;
            fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
            stream_reposition(stream,*tr);
            tr++;
            out->ascii.escape=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
            break;
        case DIRECT:
            tr = trace_begin + *tr;
            out->ascii.N_type= DIRECT;
            out->ascii.direct=read_unsigned_bits(stream,8);
            stream_advance(stream,8);
            break;
        default:
            assert("BUG");
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    case HEX:
        tr = trace_begin + *tr;
        out->N_type= HEX;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->hex.count=*(tr++);
            save = *(tr++);
            out->hex.elem= (typeof(out->hex.elem))n_malloc(arena,out->hex.count* sizeof(*out->hex.elem));
            if(!out->hex.elem) {
                return 0;
            }
            for(pos i6=0; i6<out->hex.count; i6++) {
                fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
                switch(*(tr++)) {
                case 1:
                    tr = trace_begin + *tr;
                    out->hex.elem[i6]=read_unsigned_bits(stream,8);
                    stream_advance(stream,8);
                    break;
                case 2:
                    tr = trace_begin + *tr;
                    out->hex.elem[i6]=read_unsigned_bits(stream,8);
                    stream_advance(stream,8);
                    break;
                default:
                    assert(!"Error");
                    exit(-1);
                }
            }
            tr = trace_begin + save;
        }
        break;
    case NUMBER:
        tr = trace_begin + *tr;
        out->N_type= NUMBER;
        if(parser_fail(bind_number(arena,&out->number, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
intconstant*parse_intconstant(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    intconstant* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_intconstant(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_intconstant(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_intp(NailArena *arena,intp*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case UNSIGN:
        tr = trace_begin + *tr;
        out->N_type= UNSIGN;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->unsign.count=*(tr++);
            save = *(tr++);
            out->unsign.elem= (typeof(out->unsign.elem))n_malloc(arena,out->unsign.count* sizeof(*out->unsign.elem));
            if(!out->unsign.elem) {
                return 0;
            }
            for(pos i7=0; i7<out->unsign.count; i7++) {
                out->unsign.elem[i7]=read_unsigned_bits(stream,8);
                stream_advance(stream,8);
            }
            tr = trace_begin + save;
        }
        break;
    case SIGN:
        tr = trace_begin + *tr;
        out->N_type= SIGN;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->sign.count=*(tr++);
            save = *(tr++);
            out->sign.elem= (typeof(out->sign.elem))n_malloc(arena,out->sign.count* sizeof(*out->sign.elem));
            if(!out->sign.elem) {
                return 0;
            }
            for(pos i8=0; i8<out->sign.count; i8++) {
                out->sign.elem[i8]=read_unsigned_bits(stream,8);
                stream_advance(stream,8);
            }
            tr = trace_begin + save;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
intp*parse_intp(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    intp* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_intp(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_intp(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constint(NailArena *arena,constint*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    if(parser_fail(bind_intp(arena,&out->parser, stream,&tr,trace_begin))) {
        return -1;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    if(parser_fail(bind_intconstant(arena,&out->value, stream,&tr,trace_begin))) {
        return -1;
    }*trace = tr;
    return 0;
}
constint*parse_constint(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constint* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constint(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constint(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_arrayvalue(NailArena *arena,arrayvalue*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case STRING:
        tr = trace_begin + *tr;
        out->N_type= STRING;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->string.count=*(tr++);
            save = *(tr++);
            out->string.elem= (typeof(out->string.elem))n_malloc(arena,out->string.count* sizeof(*out->string.elem));
            if(!out->string.elem) {
                return 0;
            }
            for(pos i9=0; i9<out->string.count; i9++) {
                out->string.elem[i9]=read_unsigned_bits(stream,8);
                stream_advance(stream,8);
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    case VALUES:
        tr = trace_begin + *tr;
        out->N_type= VALUES;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->values.count=*(tr++);
            save = *(tr++);
            out->values.elem= (typeof(out->values.elem))n_malloc(arena,out->values.count* sizeof(*out->values.elem));
            if(!out->values.elem) {
                return 0;
            }
            for(pos i10=0; i10<out->values.count; i10++) {
                if(parser_fail(bind_intconstant(arena,&out->values.elem[i10], stream,&tr,trace_begin))) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
arrayvalue*parse_arrayvalue(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    arrayvalue* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_arrayvalue(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_arrayvalue(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constarray(NailArena *arena,constarray*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    if(parser_fail(bind_intp(arena,&out->parser, stream,&tr,trace_begin))) {
        return -1;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    if(parser_fail(bind_arrayvalue(arena,&out->value, stream,&tr,trace_begin))) {
        return -1;
    }*trace = tr;
    return 0;
}
constarray*parse_constarray(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constarray* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constarray(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constarray(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constfields(NailArena *arena,constfields*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i11=0; i11<out->count; i11++) {
            if(i11>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
            }
            if(parser_fail(bind_constparser(arena,&out->elem[i11], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
constfields*parse_constfields(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constfields* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constfields(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constfields(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constparser(NailArena *arena,constparser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case CARRAY:
        tr = trace_begin + *tr;
        out->N_type= CARRAY;
        if(parser_fail(bind_constarray(arena,&out->carray, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CREPEAT:
        tr = trace_begin + *tr;
        out->N_type= CREPEAT;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->crepeat= (typeof(out->crepeat)) n_malloc(arena,sizeof(*out->crepeat));
        if(!out->crepeat) {
            return -1;
        }
        if(parser_fail(bind_constparser(arena,out->crepeat, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CINT:
        tr = trace_begin + *tr;
        out->N_type= CINT;
        if(parser_fail(bind_constint(arena,&out->cint, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CREF:
        tr = trace_begin + *tr;
        out->N_type= CREF;
        if(parser_fail(bind_constidentifier(arena,&out->cref, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CSTRUCT:
        tr = trace_begin + *tr;
        out->N_type= CSTRUCT;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_constfields(arena,&out->cstruct, stream,&tr,trace_begin))) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    case CUNION:
        tr = trace_begin + *tr;
        out->N_type= CUNION;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->cunion.count=*(tr++);
            save = *(tr++);
            out->cunion.elem= (typeof(out->cunion.elem))n_malloc(arena,out->cunion.count* sizeof(*out->cunion.elem));
            if(!out->cunion.elem) {
                return 0;
            }
            for(pos i12=0; i12<out->cunion.count; i12++) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
                if(parser_fail(bind_constparser(arena,&out->cunion.elem[i12], stream,&tr,trace_begin))) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
constparser*parse_constparser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constparser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constparser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constparser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constraintelem(NailArena *arena,constraintelem*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case RANGE:
        tr = trace_begin + *tr;
        out->N_type= RANGE;
        if(*tr<0) { /*OPTIONAL*/
            tr++;
            out->range.min= (typeof(out->range.min))n_malloc(arena,sizeof(*out->range.min));
            if(!out->range.min) return -1;
            if(parser_fail(bind_intconstant(arena,&out->range.min[0], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        else {
            tr = trace_begin + *tr;
            out->range.min= NULL;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(*tr<0) { /*OPTIONAL*/
            tr++;
            out->range.max= (typeof(out->range.max))n_malloc(arena,sizeof(*out->range.max));
            if(!out->range.max) return -1;
            if(parser_fail(bind_intconstant(arena,&out->range.max[0], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        else {
            tr = trace_begin + *tr;
            out->range.max= NULL;
        }
        break;
    case VALUE:
        tr = trace_begin + *tr;
        out->N_type= VALUE;
        if(parser_fail(bind_intconstant(arena,&out->value, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
constraintelem*parse_constraintelem(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constraintelem* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constraintelem(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constraintelem(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_intconstraint(NailArena *arena,intconstraint*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case SINGLE:
        tr = trace_begin + *tr;
        out->N_type= SINGLE;
        if(parser_fail(bind_constraintelem(arena,&out->single, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case SET:
        tr = trace_begin + *tr;
        out->N_type= SET;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->set.count=*(tr++);
            save = *(tr++);
            out->set.elem= (typeof(out->set.elem))n_malloc(arena,out->set.count* sizeof(*out->set.elem));
            if(!out->set.elem) {
                return 0;
            }
            for(pos i13=0; i13<out->set.count; i13++) {
                if(i13>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    stream_reposition(stream,*tr);
                    tr++;
                }
                if(parser_fail(bind_constraintelem(arena,&out->set.elem[i13], stream,&tr,trace_begin))) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    case NEGATE:
        tr = trace_begin + *tr;
        out->N_type= NEGATE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->negate= (typeof(out->negate)) n_malloc(arena,sizeof(*out->negate));
        if(!out->negate) {
            return -1;
        }
        if(parser_fail(bind_intconstraint(arena,out->negate, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
intconstraint*parse_intconstraint(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    intconstraint* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_intconstraint(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_intconstraint(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_constrainedint(NailArena *arena,constrainedint*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    if(parser_fail(bind_intp(arena,&out->parser, stream,&tr,trace_begin))) {
        return -1;
    }
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constraint= (typeof(out->constraint))n_malloc(arena,sizeof(*out->constraint));
        if(!out->constraint) return -1;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_intconstraint(arena,&out->constraint[0], stream,&tr,trace_begin))) {
            return -1;
        }
    }
    else {
        tr = trace_begin + *tr;
        out->constraint= NULL;
    }*trace = tr;
    return 0;
}
constrainedint*parse_constrainedint(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    constrainedint* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_constrainedint(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_constrainedint(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_transform(NailArena *arena,transform*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->left.count=*(tr++);
        save = *(tr++);
        out->left.elem= (typeof(out->left.elem))n_malloc(arena,out->left.count* sizeof(*out->left.elem));
        if(!out->left.elem) {
            return 0;
        }
        for(pos i14=0; i14<out->left.count; i14++) {
            if(i14>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
            }
            if(parser_fail(bind_streamidentifier(arena,&out->left.elem[i14], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    if(parser_fail(bind_varidentifier(arena,&out->cfunction, stream,&tr,trace_begin))) {
        return -1;
    }
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->right.count=*(tr++);
        save = *(tr++);
        out->right.elem= (typeof(out->right.elem))n_malloc(arena,out->right.count* sizeof(*out->right.elem));
        if(!out->right.elem) {
            return 0;
        }
        for(pos i15=0; i15<out->right.count; i15++) {
            if(parser_fail(bind_parameter(arena,&out->right.elem[i15], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }*trace = tr;
    return 0;
}
transform*parse_transform(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    transform* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_transform(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_transform(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_structparser(NailArena *arena,structparser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i16=0; i16<out->count; i16++) {
            if(i16>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
            }
            fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
            switch(*(tr++)) {
            case CONSTANT:
                tr = trace_begin + *tr;
                out->elem[i16].N_type= CONSTANT;
                if(parser_fail(bind_constparser(arena,&out->elem[i16].constant, stream,&tr,trace_begin))) {
                    return -1;
                }
                break;
            case DEPENDENCY:
                tr = trace_begin + *tr;
                out->elem[i16].N_type= DEPENDENCY;
                if(parser_fail(bind_dependencyidentifier(arena,&out->elem[i16].dependency.name, stream,&tr,trace_begin))) {
                    return -1;
                }
                out->elem[i16].dependency.parser= (typeof(out->elem[i16].dependency.parser)) n_malloc(arena,sizeof(*out->elem[i16].dependency.parser));
                if(!out->elem[i16].dependency.parser) {
                    return -1;
                }
                if(parser_fail(bind_parser(arena,out->elem[i16].dependency.parser, stream,&tr,trace_begin))) {
                    return -1;
                }
                break;
            case FIELD:
                tr = trace_begin + *tr;
                out->elem[i16].N_type= FIELD;
                if(parser_fail(bind_varidentifier(arena,&out->elem[i16].field.name, stream,&tr,trace_begin))) {
                    return -1;
                }
                out->elem[i16].field.parser= (typeof(out->elem[i16].field.parser)) n_malloc(arena,sizeof(*out->elem[i16].field.parser));
                if(!out->elem[i16].field.parser) {
                    return -1;
                }
                if(parser_fail(bind_parser(arena,out->elem[i16].field.parser, stream,&tr,trace_begin))) {
                    return -1;
                }
                break;
            case TRANSFORM:
                tr = trace_begin + *tr;
                out->elem[i16].N_type= TRANSFORM;
                if(parser_fail(bind_transform(arena,&out->elem[i16].transform, stream,&tr,trace_begin))) {
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
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
structparser*parse_structparser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    structparser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_structparser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_structparser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_wrapparser(NailArena *arena,wrapparser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constbefore= (typeof(out->constbefore))n_malloc(arena,sizeof(*out->constbefore));
        if(!out->constbefore) return -1;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->constbefore[0].count=*(tr++);
            save = *(tr++);
            out->constbefore[0].elem= (typeof(out->constbefore[0].elem))n_malloc(arena,out->constbefore[0].count* sizeof(*out->constbefore[0].elem));
            if(!out->constbefore[0].elem) {
                return 0;
            }
            for(pos i17=0; i17<out->constbefore[0].count; i17++) {
                if(i17>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    stream_reposition(stream,*tr);
                    tr++;
                }
                if(parser_fail(bind_constparser(arena,&out->constbefore[0].elem[i17], stream,&tr,trace_begin))) {
                    return -1;
                }
            }
            tr = trace_begin + save;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
    }
    else {
        tr = trace_begin + *tr;
        out->constbefore= NULL;
    }
    out->parser= (typeof(out->parser)) n_malloc(arena,sizeof(*out->parser));
    if(!out->parser) {
        return -1;
    }
    if(parser_fail(bind_parser(arena,out->parser, stream,&tr,trace_begin))) {
        return -1;
    }
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->constafter= (typeof(out->constafter))n_malloc(arena,sizeof(*out->constafter));
        if(!out->constafter) return -1;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->constafter[0].count=*(tr++);
            save = *(tr++);
            out->constafter[0].elem= (typeof(out->constafter[0].elem))n_malloc(arena,out->constafter[0].count* sizeof(*out->constafter[0].elem));
            if(!out->constafter[0].elem) {
                return 0;
            }
            for(pos i18=0; i18<out->constafter[0].count; i18++) {
                if(i18>0) {
                    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                    stream_reposition(stream,*tr);
                    tr++;
                }
                if(parser_fail(bind_constparser(arena,&out->constafter[0].elem[i18], stream,&tr,trace_begin))) {
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
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
wrapparser*parse_wrapparser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    wrapparser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_wrapparser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_wrapparser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_choiceparser(NailArena *arena,choiceparser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i19=0; i19<out->count; i19++) {
            if(parser_fail(bind_constidentifier(arena,&out->elem[i19].tag, stream,&tr,trace_begin))) {
                return -1;
            }
            fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
            stream_reposition(stream,*tr);
            tr++;
            out->elem[i19].parser= (typeof(out->elem[i19].parser)) n_malloc(arena,sizeof(*out->elem[i19].parser));
            if(!out->elem[i19].parser) {
                return -1;
            }
            if(parser_fail(bind_parser(arena,out->elem[i19].parser, stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
choiceparser*parse_choiceparser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    choiceparser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_choiceparser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_choiceparser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_arrayparser(NailArena *arena,arrayparser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case MANYONE:
        tr = trace_begin + *tr;
        out->N_type= MANYONE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->manyone= (typeof(out->manyone)) n_malloc(arena,sizeof(*out->manyone));
        if(!out->manyone) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->manyone, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case MANY:
        tr = trace_begin + *tr;
        out->N_type= MANY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->many= (typeof(out->many)) n_malloc(arena,sizeof(*out->many));
        if(!out->many) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->many, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case SEPBYONE:
        tr = trace_begin + *tr;
        out->N_type= SEPBYONE;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_constparser(arena,&out->sepbyone.separator, stream,&tr,trace_begin))) {
            return -1;
        }
        out->sepbyone.inner= (typeof(out->sepbyone.inner)) n_malloc(arena,sizeof(*out->sepbyone.inner));
        if(!out->sepbyone.inner) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->sepbyone.inner, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case SEPBY:
        tr = trace_begin + *tr;
        out->N_type= SEPBY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_constparser(arena,&out->sepby.separator, stream,&tr,trace_begin))) {
            return -1;
        }
        out->sepby.inner= (typeof(out->sepby.inner)) n_malloc(arena,sizeof(*out->sepby.inner));
        if(!out->sepby.inner) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->sepby.inner, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
arrayparser*parse_arrayparser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    arrayparser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_arrayparser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_arrayparser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parameter(NailArena *arena,parameter*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case PDEPENDENCY:
        tr = trace_begin + *tr;
        out->N_type= PDEPENDENCY;
        if(parser_fail(bind_dependencyidentifier(arena,&out->pdependency, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case PSTREAM:
        tr = trace_begin + *tr;
        out->N_type= PSTREAM;
        if(parser_fail(bind_streamidentifier(arena,&out->pstream, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
parameter*parse_parameter(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parameter* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parameter(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parameter(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parameterlist(NailArena *arena,parameterlist*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i20=0; i20<out->count; i20++) {
            if(i20>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
            }
            if(parser_fail(bind_parameter(arena,&out->elem[i20], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
parameterlist*parse_parameterlist(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parameterlist* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parameterlist(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parameterlist(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parameterdefinition(NailArena *arena,parameterdefinition*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case DSTREAM:
        tr = trace_begin + *tr;
        out->N_type= DSTREAM;
        if(parser_fail(bind_streamidentifier(arena,&out->dstream, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case DDEPENDENCY:
        tr = trace_begin + *tr;
        out->N_type= DDEPENDENCY;
        if(parser_fail(bind_dependencyidentifier(arena,&out->ddependency.name, stream,&tr,trace_begin))) {
            return -1;
        }
        out->ddependency.type= (typeof(out->ddependency.type)) n_malloc(arena,sizeof(*out->ddependency.type));
        if(!out->ddependency.type) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->ddependency.type, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
parameterdefinition*parse_parameterdefinition(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parameterdefinition* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parameterdefinition(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parameterdefinition(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parameterdefinitionlist(NailArena *arena,parameterdefinitionlist*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i21=0; i21<out->count; i21++) {
            if(i21>0) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
            }
            if(parser_fail(bind_parameterdefinition(arena,&out->elem[i21], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
parameterdefinitionlist*parse_parameterdefinitionlist(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parameterdefinitionlist* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parameterdefinitionlist(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parameterdefinitionlist(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parserinvocation(NailArena *arena,parserinvocation*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    if(parser_fail(bind_varidentifier(arena,&out->name, stream,&tr,trace_begin))) {
        return -1;
    }
    if(*tr<0) { /*OPTIONAL*/
        tr++;
        out->parameters= (typeof(out->parameters))n_malloc(arena,sizeof(*out->parameters));
        if(!out->parameters) return -1;
        if(parser_fail(bind_parameterlist(arena,&out->parameters[0], stream,&tr,trace_begin))) {
            return -1;
        }
    }
    else {
        tr = trace_begin + *tr;
        out->parameters= NULL;
    }*trace = tr;
    return 0;
}
parserinvocation*parse_parserinvocation(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parserinvocation* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parserinvocation(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parserinvocation(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parserinner(NailArena *arena,parserinner*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case INTEGER:
        tr = trace_begin + *tr;
        out->N_type= INTEGER;
        if(parser_fail(bind_constrainedint(arena,&out->integer, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case STRUCTURE:
        tr = trace_begin + *tr;
        out->N_type= STRUCTURE;
        if(parser_fail(bind_structparser(arena,&out->structure, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case WRAP:
        tr = trace_begin + *tr;
        out->N_type= WRAP;
        if(parser_fail(bind_wrapparser(arena,&out->wrap, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CHOICE:
        tr = trace_begin + *tr;
        out->N_type= CHOICE;
        if(parser_fail(bind_choiceparser(arena,&out->choice, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case ARRAY:
        tr = trace_begin + *tr;
        out->N_type= ARRAY;
        if(parser_fail(bind_arrayparser(arena,&out->array, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case FIXEDARRAY:
        tr = trace_begin + *tr;
        out->N_type= FIXEDARRAY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_intconstant(arena,&out->fixedarray.length, stream,&tr,trace_begin))) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->fixedarray.inner= (typeof(out->fixedarray.inner)) n_malloc(arena,sizeof(*out->fixedarray.inner));
        if(!out->fixedarray.inner) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->fixedarray.inner, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case LENGTH:
        tr = trace_begin + *tr;
        out->N_type= LENGTH;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_dependencyidentifier(arena,&out->length.length, stream,&tr,trace_begin))) {
            return -1;
        }
        out->length.parser= (typeof(out->length.parser)) n_malloc(arena,sizeof(*out->length.parser));
        if(!out->length.parser) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->length.parser, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case OPTIONAL:
        tr = trace_begin + *tr;
        out->N_type= OPTIONAL;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        out->optional= (typeof(out->optional)) n_malloc(arena,sizeof(*out->optional));
        if(!out->optional) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->optional, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case NUNION:
        tr = trace_begin + *tr;
        out->N_type= NUNION;
        fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
        {   /*ARRAY*/
            pos save = 0;
            out->nunion.count=*(tr++);
            save = *(tr++);
            out->nunion.elem= (typeof(out->nunion.elem))n_malloc(arena,out->nunion.count* sizeof(*out->nunion.elem));
            if(!out->nunion.elem) {
                return 0;
            }
            for(pos i22=0; i22<out->nunion.count; i22++) {
                fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
                stream_reposition(stream,*tr);
                tr++;
                out->nunion.elem[i22]= (typeof(out->nunion.elem[i22])) n_malloc(arena,sizeof(*out->nunion.elem[i22]));
                if(!out->nunion.elem[i22]) {
                    return -1;
                }
                if(parser_fail(bind_parser(arena,out->nunion.elem[i22], stream,&tr,trace_begin))) {
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
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_parserinvocation(arena,&out->ref, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case NAME:
        tr = trace_begin + *tr;
        out->N_type= NAME;
        if(parser_fail(bind_parserinvocation(arena,&out->name, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case APPLY:
        tr = trace_begin + *tr;
        out->N_type= APPLY;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_streamidentifier(arena,&out->apply.stream, stream,&tr,trace_begin))) {
            return -1;
        }
        out->apply.inner= (typeof(out->apply.inner)) n_malloc(arena,sizeof(*out->apply.inner));
        if(!out->apply.inner) {
            return -1;
        }
        if(parser_fail(bind_parser(arena,out->apply.inner, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
parserinner*parse_parserinner(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parserinner* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parserinner(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parserinner(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_parser(NailArena *arena,parser*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case PAREN:
        tr = trace_begin + *tr;
        out->N_type= PAREN;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_parserinner(arena,&out->paren, stream,&tr,trace_begin))) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        break;
    case PR:
        tr = trace_begin + *tr;
        out->N_type= PR;
        if(parser_fail(bind_parserinner(arena,&out->pr, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
parser*parse_parser(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    parser* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_parser(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_parser(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_definition(NailArena *arena,definition*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
    switch(*(tr++)) {
    case PARSER:
        tr = trace_begin + *tr;
        out->N_type= PARSER;
        if(parser_fail(bind_varidentifier(arena,&out->parser.name, stream,&tr,trace_begin))) {
            return -1;
        }
        if(*tr<0) { /*OPTIONAL*/
            tr++;
            out->parser.parameters= (typeof(out->parser.parameters))n_malloc(arena,sizeof(*out->parser.parameters));
            if(!out->parser.parameters) return -1;
            if(parser_fail(bind_parameterdefinitionlist(arena,&out->parser.parameters[0], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        else {
            tr = trace_begin + *tr;
            out->parser.parameters= NULL;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_parser(arena,&out->parser.definition, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    case CONSTANTDEF:
        tr = trace_begin + *tr;
        out->N_type= CONSTANTDEF;
        if(parser_fail(bind_constidentifier(arena,&out->constantdef.name, stream,&tr,trace_begin))) {
            return -1;
        }
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
        stream_reposition(stream,*tr);
        tr++;
        if(parser_fail(bind_constparser(arena,&out->constantdef.definition, stream,&tr,trace_begin))) {
            return -1;
        }
        break;
    default:
        assert("BUG");
    }*trace = tr;
    return 0;
}
definition*parse_definition(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    definition* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_definition(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_definition(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
static int bind_grammar(NailArena *arena,grammar*out,NailStream *stream, pos **trace ,  pos * trace_begin) {
    pos *tr = *trace;
    fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
    {   /*ARRAY*/
        pos save = 0;
        out->count=*(tr++);
        save = *(tr++);
        out->elem= (typeof(out->elem))n_malloc(arena,out->count* sizeof(*out->elem));
        if(!out->elem) {
            return 0;
        }
        for(pos i23=0; i23<out->count; i23++) {
            if(parser_fail(bind_definition(arena,&out->elem[i23], stream,&tr,trace_begin))) {
                return -1;
            }
        }
        tr = trace_begin + save;
    }
    fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
    stream_reposition(stream,*tr);
    tr++;
    *trace = tr;
    return 0;
}
grammar*parse_grammar(NailArena *arena, const uint8_t *data, size_t size) {
    NailStream stream = {.data = data, .pos= 0, .size = size*8};
    n_trace trace;
    pos *tr_ptr;
    pos pos;
    grammar* retval;
    n_trace_init(&trace,4096,4096);
    tr_ptr = trace.trace;
    if(size*8 != peg_grammar(&trace,&stream)) return NULL;
    retval =  (typeof(retval))n_malloc(arena,sizeof(*retval));
    if(!retval) return NULL;
    stream.pos = 0;
    if(bind_grammar(arena,retval,&stream,&tr_ptr,trace.trace) < 0) return NULL;
    n_trace_release(&trace);
    return retval;
}
#include <hammer/hammer.h>
#include <hammer/internal.h>
void gen_number(HBitWriter *out,number * val);
void gen_varidentifier(HBitWriter *out,varidentifier * val);
void gen_constidentifier(HBitWriter *out,constidentifier * val);
void gen_streamidentifier(HBitWriter *out,streamidentifier * val);
void gen_dependencyidentifier(HBitWriter *out,dependencyidentifier * val);
void gen_WHITE(HBitWriter* out);
void gen_SEPERATOR(HBitWriter* out);
void gen_intconstant(HBitWriter *out,intconstant * val);
void gen_intp(HBitWriter *out,intp * val);
void gen_constint(HBitWriter *out,constint * val);
void gen_arrayvalue(HBitWriter *out,arrayvalue * val);
void gen_constarray(HBitWriter *out,constarray * val);
void gen_constfields(HBitWriter *out,constfields * val);
void gen_constparser(HBitWriter *out,constparser * val);
void gen_constraintelem(HBitWriter *out,constraintelem * val);
void gen_intconstraint(HBitWriter *out,intconstraint * val);
void gen_constrainedint(HBitWriter *out,constrainedint * val);
void gen_transform(HBitWriter *out,transform * val);
void gen_structparser(HBitWriter *out,structparser * val);
void gen_wrapparser(HBitWriter *out,wrapparser * val);
void gen_choiceparser(HBitWriter *out,choiceparser * val);
void gen_arrayparser(HBitWriter *out,arrayparser * val);
void gen_parameter(HBitWriter *out,parameter * val);
void gen_parameterlist(HBitWriter *out,parameterlist * val);
void gen_parameterdefinition(HBitWriter *out,parameterdefinition * val);
void gen_parameterdefinitionlist(HBitWriter *out,parameterdefinitionlist * val);
void gen_parserinvocation(HBitWriter *out,parserinvocation * val);
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
void gen_streamidentifier(HBitWriter *out,streamidentifier * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'#',8);
    for(int i3=0; i3<val->count; i3++) {
        h_bit_writer_put(out,val->elem[i3],8);
    }
}
void gen_dependencyidentifier(HBitWriter *out,dependencyidentifier * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'@',8);
    for(int i4=0; i4<val->count; i4++) {
        h_bit_writer_put(out,val->elem[i4],8);
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
        switch(val->ascii.N_type) {
        case ESCAPE:
            h_bit_writer_put(out,'\\',8);
            h_bit_writer_put(out,val->ascii.escape,8);
            break;
        case DIRECT:
            h_bit_writer_put(out,val->ascii.direct,8);
            break;
        }
        h_bit_writer_put(out,'\'',8);
        break;
    case HEX:
        h_bit_writer_put(out,'0',8);
        h_bit_writer_put(out,'x',8);
        for(int i5=0; i5<val->hex.count; i5++) {
            h_bit_writer_put(out,val->hex.elem[i5],8);
        }
        break;
    case NUMBER:
        gen_number(out,&val->number);
        break;
    }
}
void gen_intp(HBitWriter *out,intp * val) {
    switch(val->N_type) {
    case UNSIGN:
        gen_WHITE(out);
        h_bit_writer_put(out,'u',8);
        h_bit_writer_put(out,'i',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'t',8);
        for(int i6=0; i6<val->unsign.count; i6++) {
            h_bit_writer_put(out,val->unsign.elem[i6],8);
        }
        break;
    case SIGN:
        gen_WHITE(out);
        h_bit_writer_put(out,'i',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'t',8);
        for(int i7=0; i7<val->sign.count; i7++) {
            h_bit_writer_put(out,val->sign.elem[i7],8);
        }
        break;
    }
}
void gen_constint(HBitWriter *out,constint * val) {
    gen_intp(out,&val->parser);
    gen_WHITE(out);
    h_bit_writer_put(out,'=',8);
    gen_intconstant(out,&val->value);
    {/*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
}
void gen_arrayvalue(HBitWriter *out,arrayvalue * val) {
    switch(val->N_type) {
    case STRING:
        gen_WHITE(out);
        h_bit_writer_put(out,'"',8);
        for(int i8=0; i8<val->string.count; i8++) {
            h_bit_writer_put(out,val->string.elem[i8],8);
        }
        h_bit_writer_put(out,'"',8);
        break;
    case VALUES:
        gen_WHITE(out);
        h_bit_writer_put(out,'[',8);
        for(int i9=0; i9<val->values.count; i9++) {
            gen_intconstant(out,&val->values.elem[i9]);
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
    {/*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
}
void gen_constfields(HBitWriter *out,constfields * val) {
    for(int i10=0; i10<val->count; i10++) {
        if(i10!= 0) {
            gen_SEPERATOR(out);
        }
        gen_constparser(out,&val->elem[i10]);
    }
}
void gen_constparser(HBitWriter *out,constparser * val) {
    switch(val->N_type) {
    case CARRAY:
        gen_constarray(out,&val->carray);
        break;
    case CREPEAT:
        gen_WHITE(out);
        h_bit_writer_put(out,'m',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'y',8);
        h_bit_writer_put(out,' ',8);
        gen_constparser(out,val->crepeat);
        break;
    case CINT:
        gen_constint(out,&val->cint);
        break;
    case CREF:
        gen_constidentifier(out,&val->cref);
        break;
    case CSTRUCT:
        gen_WHITE(out);
        h_bit_writer_put(out,'{',8);
        gen_constfields(out,&val->cstruct);
        gen_WHITE(out);
        h_bit_writer_put(out,'}',8);
        break;
    case CUNION:
        for(int i11=0; i11<val->cunion.count; i11++) {
            gen_WHITE(out);
            h_bit_writer_put(out,'|',8);
            h_bit_writer_put(out,'|',8);
            gen_constparser(out,&val->cunion.elem[i11]);
        }
        break;
    }
}
void gen_constraintelem(HBitWriter *out,constraintelem * val) {
    switch(val->N_type) {
    case RANGE:
        if(NULL!=val->range.min) {
            gen_intconstant(out,&val->range.min[0]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,'.',8);
        h_bit_writer_put(out,'.',8);
        if(NULL!=val->range.max) {
            gen_intconstant(out,&val->range.max[0]);
        }{/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    case VALUE:
        gen_intconstant(out,&val->value);
        break;
    }
}
void gen_intconstraint(HBitWriter *out,intconstraint * val) {
    switch(val->N_type) {
    case SINGLE:
        gen_constraintelem(out,&val->single);
        break;
    case SET:
        gen_WHITE(out);
        h_bit_writer_put(out,'[',8);
        for(int i12=0; i12<val->set.count; i12++) {
            if(i12!= 0) {
                h_bit_writer_put(out,',',8);
            }
            gen_constraintelem(out,&val->set.elem[i12]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,']',8);
        break;
    case NEGATE:
        gen_WHITE(out);
        h_bit_writer_put(out,'!',8);
        gen_intconstraint(out,val->negate);
        break;
    }
}
void gen_constrainedint(HBitWriter *out,constrainedint * val) {
    gen_intp(out,&val->parser);
    if(NULL!=val->constraint) {
        gen_WHITE(out);
        h_bit_writer_put(out,'|',8);
        gen_intconstraint(out,&val->constraint[0]);
    }{/*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
}
void gen_transform(HBitWriter *out,transform * val) {
    for(int i13=0; i13<val->left.count; i13++) {
        if(i13!= 0) {
            gen_WHITE(out);
            h_bit_writer_put(out,',',8);
        }
        gen_streamidentifier(out,&val->left.elem[i13]);
    }
    gen_WHITE(out);
    h_bit_writer_put(out,'t',8);
    h_bit_writer_put(out,'r',8);
    h_bit_writer_put(out,'a',8);
    h_bit_writer_put(out,'n',8);
    h_bit_writer_put(out,'s',8);
    h_bit_writer_put(out,'f',8);
    h_bit_writer_put(out,'o',8);
    h_bit_writer_put(out,'r',8);
    h_bit_writer_put(out,'m',8);
    gen_varidentifier(out,&val->cfunction);
    for(int i14=0; i14<val->right.count; i14++) {
        gen_parameter(out,&val->right.elem[i14]);
    }{/*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
}
void gen_structparser(HBitWriter *out,structparser * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'{',8);
    for(int i15=0; i15<val->count; i15++) {
        if(i15!= 0) {
            gen_SEPERATOR(out);
        }
        switch(val->elem[i15].N_type) {
        case CONSTANT:
            gen_constparser(out,&val->elem[i15].constant);
            break;
        case DEPENDENCY:
            gen_dependencyidentifier(out,&val->elem[i15].dependency.name);
            gen_parser(out,val->elem[i15].dependency.parser);
            {/*Context-rewind*/
                HBitWriter end_of_struct= *out;
                out->index = end_of_struct.index;
                out->bit_offset = end_of_struct.bit_offset;
            }
            break;
        case FIELD:
            gen_varidentifier(out,&val->elem[i15].field.name);
            gen_parser(out,val->elem[i15].field.parser);
            {/*Context-rewind*/
                HBitWriter end_of_struct= *out;
                out->index = end_of_struct.index;
                out->bit_offset = end_of_struct.bit_offset;
            }
            break;
        case TRANSFORM:
            gen_transform(out,&val->elem[i15].transform);
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
        for(int i16=0; i16<val->constbefore[0].count; i16++) {
            if(i16!= 0) {
                gen_SEPERATOR(out);
            }
            gen_constparser(out,&val->constbefore[0].elem[i16]);
        }
        gen_SEPERATOR(out);
    }
    gen_parser(out,val->parser);
    if(NULL!=val->constafter) {
        gen_SEPERATOR(out);
        for(int i17=0; i17<val->constafter[0].count; i17++) {
            if(i17!= 0) {
                gen_SEPERATOR(out);
            }
            gen_constparser(out,&val->constafter[0].elem[i17]);
        }
    }
    gen_WHITE(out);
    h_bit_writer_put(out,'>',8);
    {   /*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
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
    for(int i18=0; i18<val->count; i18++) {
        gen_constidentifier(out,&val->elem[i18].tag);
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_parser(out,val->elem[i18].parser);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
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
        gen_parser(out,val->manyone);
        break;
    case MANY:
        gen_WHITE(out);
        h_bit_writer_put(out,'m',8);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'y',8);
        gen_parser(out,val->many);
        break;
    case SEPBYONE:
        gen_WHITE(out);
        h_bit_writer_put(out,'s',8);
        h_bit_writer_put(out,'e',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'B',8);
        h_bit_writer_put(out,'y',8);
        h_bit_writer_put(out,'1',8);
        gen_constparser(out,&val->sepbyone.separator);
        gen_parser(out,val->sepbyone.inner);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    case SEPBY:
        gen_WHITE(out);
        h_bit_writer_put(out,'s',8);
        h_bit_writer_put(out,'e',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'B',8);
        h_bit_writer_put(out,'y',8);
        gen_constparser(out,&val->sepby.separator);
        gen_parser(out,val->sepby.inner);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    }
}
void gen_parameter(HBitWriter *out,parameter * val) {
    switch(val->N_type) {
    case PDEPENDENCY:
        gen_dependencyidentifier(out,&val->pdependency);
        break;
    case PSTREAM:
        gen_streamidentifier(out,&val->pstream);
        break;
    }
}
void gen_parameterlist(HBitWriter *out,parameterlist * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'(',8);
    for(int i19=0; i19<val->count; i19++) {
        if(i19!= 0) {
            gen_WHITE(out);
            h_bit_writer_put(out,',',8);
        }
        gen_parameter(out,&val->elem[i19]);
    }
    gen_WHITE(out);
    h_bit_writer_put(out,')',8);
}
void gen_parameterdefinition(HBitWriter *out,parameterdefinition * val) {
    switch(val->N_type) {
    case DSTREAM:
        gen_streamidentifier(out,&val->dstream);
        break;
    case DDEPENDENCY:
        gen_dependencyidentifier(out,&val->ddependency.name);
        gen_parser(out,val->ddependency.type);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    }
}
void gen_parameterdefinitionlist(HBitWriter *out,parameterdefinitionlist * val) {
    gen_WHITE(out);
    h_bit_writer_put(out,'(',8);
    for(int i20=0; i20<val->count; i20++) {
        if(i20!= 0) {
            gen_WHITE(out);
            h_bit_writer_put(out,',',8);
        }
        gen_parameterdefinition(out,&val->elem[i20]);
    }
    gen_WHITE(out);
    h_bit_writer_put(out,')',8);
}
void gen_parserinvocation(HBitWriter *out,parserinvocation * val) {
    gen_varidentifier(out,&val->name);
    if(NULL!=val->parameters) {
        gen_parameterlist(out,&val->parameters[0]);
    }{/*Context-rewind*/
        HBitWriter end_of_struct= *out;
        out->index = end_of_struct.index;
        out->bit_offset = end_of_struct.bit_offset;
    }
}
void gen_parserinner(HBitWriter *out,parserinner * val) {
    switch(val->N_type) {
    case INTEGER:
        gen_constrainedint(out,&val->integer);
        break;
    case STRUCTURE:
        gen_structparser(out,&val->structure);
        break;
    case WRAP:
        gen_wrapparser(out,&val->wrap);
        break;
    case CHOICE:
        gen_choiceparser(out,&val->choice);
        break;
    case ARRAY:
        gen_arrayparser(out,&val->array);
        break;
    case FIXEDARRAY:
        gen_WHITE(out);
        h_bit_writer_put(out,'[',8);
        gen_intconstant(out,&val->fixedarray.length);
        gen_WHITE(out);
        h_bit_writer_put(out,']',8);
        gen_parser(out,val->fixedarray.inner);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    case LENGTH:
        gen_WHITE(out);
        h_bit_writer_put(out,'n',8);
        h_bit_writer_put(out,'_',8);
        h_bit_writer_put(out,'o',8);
        h_bit_writer_put(out,'f',8);
        gen_dependencyidentifier(out,&val->length.length);
        gen_parser(out,val->length.parser);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
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
        gen_parser(out,val->optional);
        break;
    case NUNION:
        for(int i21=0; i21<val->nunion.count; i21++) {
            gen_WHITE(out);
            h_bit_writer_put(out,'|',8);
            h_bit_writer_put(out,'|',8);
            gen_parser(out,val->nunion.elem[i21]);
        }
        break;
    case REF:
        gen_WHITE(out);
        h_bit_writer_put(out,'*',8);
        gen_parserinvocation(out,&val->ref);
        break;
    case NAME:
        gen_parserinvocation(out,&val->name);
        break;
    case APPLY:
        gen_WHITE(out);
        h_bit_writer_put(out,'a',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'p',8);
        h_bit_writer_put(out,'l',8);
        h_bit_writer_put(out,'y',8);
        gen_streamidentifier(out,&val->apply.stream);
        gen_parser(out,val->apply.inner);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    }
}
void gen_parser(HBitWriter *out,parser * val) {
    switch(val->N_type) {
    case PAREN:
        gen_WHITE(out);
        h_bit_writer_put(out,'(',8);
        gen_parserinner(out,&val->paren);
        gen_WHITE(out);
        h_bit_writer_put(out,')',8);
        break;
    case PR:
        gen_parserinner(out,&val->pr);
        break;
    }
}
void gen_definition(HBitWriter *out,definition * val) {
    switch(val->N_type) {
    case PARSER:
        gen_varidentifier(out,&val->parser.name);
        if(NULL!=val->parser.parameters) {
            gen_parameterdefinitionlist(out,&val->parser.parameters[0]);
        }
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_parser(out,&val->parser.definition);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    case CONSTANTDEF:
        gen_constidentifier(out,&val->constantdef.name);
        gen_WHITE(out);
        h_bit_writer_put(out,'=',8);
        gen_WHITE(out);
        gen_constparser(out,&val->constantdef.definition);
        {/*Context-rewind*/
            HBitWriter end_of_struct= *out;
            out->index = end_of_struct.index;
            out->bit_offset = end_of_struct.bit_offset;
        }
        break;
    }
}
void gen_grammar(HBitWriter *out,grammar * val) {
    for(int i22=0; i22<val->count; i22++) {
        gen_definition(out,&val->elem[i22]);
    }
    gen_WHITE(out);
}

