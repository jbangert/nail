#include "dns.nail.h"
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
#define parser_fail(i) __builtin_expect(i,0)
static uint64_t read_unsigned_bits_littleendian(NailStream *stream, unsigned count) {
    uint64_t retval = 0;
    unsigned int out_idx=0;
    size_t pos = stream->pos;
    char bit_offset = stream->bit_offset;
    const uint8_t *data = stream->data;
    while(count>0) {
        if(bit_offset == 0 && (count &7) ==0) {
            retval|= data[pos] << out_idx;
            out_idx+=8;
            pos ++;
            count-=8;
        }
        else {
            //This can use a lot of performance love
//TODO: test this
            retval |= ((data[pos] >> (bit_offset)) & 1) << out_idx;
            out_idx++;
            count--;
            bit_offset++;
            if(bit_offset >7) {
                bit_offset -= 8;
                pos++;
            }
        }
    }
    stream->pos = pos;
    stream->bit_offset = bit_offset;
    return retval;

}

static uint64_t read_unsigned_bits(NailStream *stream, unsigned count) {
    uint64_t retval = 0;
    unsigned int out_idx=count;
    size_t pos = stream->pos;
    char bit_offset = stream->bit_offset;
    const uint8_t *data = stream->data;
    //TODO: Implement little endian too
    //Count LSB to MSB
    while(count>0) {
        if(bit_offset == 0 && (count &7) ==0) {
            out_idx-=8;
            retval|= data[pos] << out_idx;
            pos ++;
            count-=8;
        }
        else {
            //This can use a lot of performance love
//TODO: implement other endianesses
            out_idx--;
            retval |= ((data[pos] >> (7-bit_offset)) & 1) << out_idx;
            count--;
            bit_offset++;
            if(bit_offset > 7) {
                bit_offset -= 8;
                pos++;
            }
        }
    }
    stream->pos = pos;
    stream->bit_offset = bit_offset;
    return retval;
}
static int stream_check(const NailStream *stream, unsigned count) {
    if(stream->size - (count>>3) - ((stream->bit_offset + count & 7)>>3) < stream->pos)
        return -1;
    return 0;
}
static void stream_advance(NailStream *stream, unsigned count) {

    stream->pos += count >> 3;
    stream->bit_offset += count &7;
    if(stream->bit_offset > 7) {
        stream->pos++;
        stream->bit_offset-=8;
    }
}
static void stream_backup(NailStream *stream, unsigned count) {
    stream->pos -= count >> 3;
    stream->bit_offset -= count & 7;
    if(stream->bit_offset < 0) {
        stream->pos--;
        stream->bit_offset += 8;
    }
}
static int stream_reposition(NailStream *stream, NailStreamPos p)
{
    stream->pos = p >> 3;
    stream->bit_offset = p & 7;
    return 0;
}
static NailStreamPos   stream_getpos(NailStream *stream) {
    return stream->pos << 3 + stream->bit_offset; //TODO: Overflow potential!
}

int NailOutStream_init(NailStream *out,size_t siz) {
    out->data = (const uint8_t *)malloc(siz);
    if(!out->data)
        return -1;
    out->pos = 0;
    out->bit_offset = 0;
    out->size = siz;
    return 0;
}
void NailOutStream_release(NailStream *out) {
    free(out->data);
    out->data = NULL;
}
const uint8_t * NailOutStream_buffer(NailStream *str,size_t *siz) {
    if(str->bit_offset)
        return NULL;
    *siz =  str->pos;
    return str->data;
}
//TODO: Perhaps use a separate structure for output streams?
int NailOutStream_grow(NailStream *stream, size_t count) {
    if(stream->pos + count>>3 + 1 >= stream->size) {
        //TODO: parametrize stream growth
        int alloc_size = stream->pos + count>>3 + 1;
        if(4096+stream->size>alloc_size) alloc_size = 4096+stream->size;
        stream->data = realloc((void *)stream->data,alloc_size);
        stream->size = alloc_size;
        if(!stream->data)
            return -1;
    }
    return 0;
}
static int stream_output(NailStream *stream,uint64_t data, size_t count) {
    if(parser_fail(NailOutStream_grow(stream, count))) {
        return -1;
    }
    uint8_t *streamdata = (uint8_t *)stream->data;
    while(count>0) {
        if(stream->bit_offset == 0 && (count & 7) == 0) {
            count -= 8;
            streamdata[stream->pos] = (data >> count );
            stream->pos++;
        }
        else {
            count--;
            if((data>>count)&1)
                streamdata[stream->pos] |= 1 << (7-stream->bit_offset);
            else
                streamdata[stream->pos] &= ~(1<< (7-stream->bit_offset));
            stream->bit_offset++;
            if(stream->bit_offset>7) {
                stream->pos++;
                stream->bit_offset= 0;
            }
        }
    }
    return 0;
}
//#define BITSLICE(x, off, len) read_unsigned_bits(x,off,len)
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
#if 0
static int  n_trace_init(n_trace *out,pos size,pos grow) {
    if(size <= 1) {
        return -1;
    }
    out->trace = (pos *)malloc(size * sizeof(pos));
    if(!out) {
        return -1;
    }
    out->capacity = size -1 ;
    out->iter = 0;
    if(grow < 16) { // Grow needs to be at least 2, but small grow makes no sense
        grow = 16;
    }
    out->grow = grow;
    return 0;
}
static void n_trace_release(n_trace *out) {
    free(out->trace);
    out->trace =NULL;
    out->capacity = 0;
    out->iter = 0;
    out->grow = 0;
}
static pos n_trace_getpos(n_trace *tr) {
    return tr->iter;
}
static void n_tr_setpos(n_trace *tr,pos offset) {
    assert(offset<tr->capacity);
    tr->iter = offset;
}
static int n_trace_grow(n_trace *out, int space) {
    if(out->capacity - space>= out->iter) {
        return 0;
    }

    pos * new_ptr= (pos *)realloc(out->trace, (out->capacity + out->grow) * sizeof(pos));
    if(!new_ptr) {
        return -1;
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
    if(parser_fail(n_trace_grow(trace,2)))
        return -1;
    trace->trace[trace->iter] = 0xFFFFFFFE;
    trace->trace[trace->iter+1] = 0xEEEEEEEF;
    trace->iter +=2;
    return trace->iter-2;

}
static void n_tr_write_many(n_trace *trace, pos where, pos count) {
    trace->trace[where] = count;
    trace->trace[where+1] = trace->iter;
#ifdef NAIL_DEBUG
    fprintf(stderr,"%d = many %d %d\n", where, count,trace->iter);
#endif
}

static pos n_tr_begin_choice(n_trace *trace) {
    if(parser_fail(n_trace_grow(trace,2)))
        return -1;

    //Debugging values
    trace->trace[trace->iter] = 0xFFFFFFFF;
    trace->trace[trace->iter+1] = 0xEEEEEEEE;
    trace->iter+= 2;
    return trace->iter - 2;
}
static int n_tr_stream(n_trace *trace, const NailStream *stream) {
    assert(sizeof(stream) % sizeof(pos) == 0);
    if(parser_fail(n_trace_grow(trace,sizeof(*stream)/sizeof(pos))))
        return -1;
    *(NailStream *)(trace->trace + trace->iter) = *stream;
#ifdef NAIL_DEBUG
    fprintf(stderr,"%d = stream\n",trace->iter,stream);
#endif
    trace->iter += sizeof(*stream)/sizeof(pos);
    return 0;
}
static pos n_tr_memo_choice(n_trace *trace) {
    return trace->iter;
}
static void n_tr_pick_choice(n_trace *trace, pos where, pos which_choice, pos  choice_begin) {
    trace->trace[where] = which_choice;
    trace->trace[where + 1] = choice_begin;
#ifdef NAIL_DEBUG
    fprintf(stderr,"%d = pick %d %d\n",where, which_choice,choice_begin);
#endif
}
static int n_tr_const(n_trace *trace,NailStream *stream) {
    if(parser_fail(n_trace_grow(trace,1)))
        return -1;
    NailStreamPos newoff = stream_getpos(stream);
#ifdef NAIL_DEBUG
    fprintf(stderr,"%d = const %d \n",trace->iter, newoff);
#endif
    trace->trace[trace->iter++] = newoff;
    return 0;
}
#endif
#define n_tr_offset n_tr_const
typedef struct NailArenaPool {
    char *iter;
    char *end;
    struct NailArenaPool *next;
} NailArenaPool;

// free on backtrack?
typedef struct {
    struct NailArenaPool *pool;
    char *iter;
} NailArenaPos;
static NailArenaPos n_arena_save(NailArena *arena) {
    NailArenaPos retval = {.pool = arena->current, .iter = arena->current->iter};
    return retval;
}
static void n_arena_restore(NailArena *arena, NailArenaPos p) {
    arena->current = p.pool;
    arena->current->iter = p.iter;
    //memory will remain linked
}
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




extern int dnscompress_parse(NailArena *tmp,NailStream *str_decompressed,NailStream *current);
static pos peg_labels(NailArena *arena,NailArena *tmparena,labels *out,NailStream *str_current);
static pos peg_compressed(NailArena *arena,NailArena *tmparena,compressed *out,NailStream *str_current);
static pos peg_question(NailArena *arena,NailArena *tmparena,question *out,NailStream *str_current);
static pos peg_answer(NailArena *arena,NailArena *tmparena,answer *out,NailStream *str_current);
static pos peg_dnspacket(NailArena *arena,NailArena *tmparena,dnspacket *out,NailStream *str_current);
static pos peg_WHITE(NailStream *str_current);
static pos peg_number(NailArena *arena,NailArena *tmparena,number *out,NailStream *str_current);
static pos peg_ipaddr(NailArena *arena,NailArena *tmparena,ipaddr *out,NailStream *str_current);
static pos peg_alnum(NailArena *arena,NailArena *tmparena,alnum *out,NailStream *str_current);
static pos peg_domain(NailArena *arena,NailArena *tmparena,domain *out,NailStream *str_current);
static pos peg_definition(NailArena *arena,NailArena *tmparena,definition *out,NailStream *str_current);
static pos peg_zone(NailArena *arena,NailArena *tmparena,zone *out,NailStream *str_current);
static pos peg_labels(NailArena *arena,NailArena *tmparena,labels *out,NailStream *str_current) {
    pos i;
    {
        pos count_1= 0;
        struct {
            typeof(out->elem[count_1]) elem;
            void *prev;
        } *tmp_1 = NULL;
succ_repeat_1:
        ;
        NailStreamPos posrep_1= stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        typeof(tmp_1) prev_tmp_1= tmp_1;
        tmp_1 = n_malloc(tmparena,sizeof(*tmp_1));
        if(parser_fail(!tmp_1)) {
            return -1;
        }
        tmp_1->prev = prev_tmp_1;
        uint8_t dep_length;
        if(parser_fail(stream_check(str_current,8))) {
            goto fail_repeat_1;
        }
        {
            dep_length = read_unsigned_bits(str_current,8);
            if((dep_length>64||dep_length<1)) {
                goto fail_repeat_1;
            }
        }
        pos i2 = 0 ;
        tmp_1[0].elem.label.count= dep_length;
        tmp_1[0].elem.label.elem = n_malloc(arena,tmp_1[0].elem.label.count*sizeof(tmp_1[0].elem.label.elem[i2]));
        if(parser_fail(!tmp_1[0].elem.label.elem)) {
            return -1;
        }
        for(; i2<tmp_1[0].elem.label.count; i2++) {
            if(parser_fail(stream_check(str_current,8))) {
                goto fail_repeat_1;
            }
            {
                tmp_1[0].elem.label.elem[i2] = read_unsigned_bits(str_current,8);
            }
        }
        count_1++;
        goto succ_repeat_1;
fail_repeat_1:
        tmp_1= tmp_1->prev;
fail_repeat_sep_1:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, posrep_1);
        out->count= count_1;
        out->elem= n_malloc(arena,sizeof(out->elem[count_1])*out->count);
        if(parser_fail(!out->elem)) {
            return -1;
        }
        while(count_1) {
            count_1--;
            memcpy(&out->elem[count_1],&tmp_1->elem,sizeof(out->elem[count_1]));
            tmp_1 = tmp_1->prev;
        }
    }
    if(parser_fail(stream_check(str_current,8))) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,8)!= 0) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
labels* parse_labels(NailArena *arena, NailStream *stream) {
    labels*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_labels(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_compressed(NailArena *arena,NailArena *tmparena,compressed *out,NailStream *str_current) {
    pos i;
    ;
    NailStream str_decompressed;
    if(dnscompress_parse(tmparena, &str_decompressed,str_current)) {
        goto fail;
    }{/*APPLY*/NailStream * origstr_3 = str_current;
        NailStreamPos back_3= stream_getpos(str_current);
        str_current = &str_decompressed;
        if(parser_fail(peg_labels(arena,tmparena, &out->labels,str_current ))) {
            goto fail_apply_3;
        }
        goto succ_apply_3;
fail_apply_3:
        str_current = origstr_3;
        stream_reposition(str_current, back_3);
        goto fail;
succ_apply_3:
        str_current = origstr_3;
    }
    return 0;
fail:
    return -1;
}
compressed* parse_compressed(NailArena *arena, NailStream *stream) {
    compressed*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_compressed(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_question(NailArena *arena,NailArena *tmparena,question *out,NailStream *str_current) {
    pos i;
    if(parser_fail(peg_compressed(arena,tmparena, &out->labels,str_current ))) {
        goto fail;
    }
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        out->qtype = read_unsigned_bits(str_current,16);
        if((out->qtype>16||out->qtype<1)) {
            goto fail;
        }
    }
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        out->qclass = read_unsigned_bits(str_current,16);
        if(out->qclass!=1 && out->qclass!=255) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
question* parse_question(NailArena *arena, NailStream *stream) {
    question*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_question(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_answer(NailArena *arena,NailArena *tmparena,answer *out,NailStream *str_current) {
    pos i;
    if(parser_fail(peg_compressed(arena,tmparena, &out->labels,str_current ))) {
        goto fail;
    }
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        out->rtype = read_unsigned_bits(str_current,16);
        if((out->rtype>16||out->rtype<1)) {
            goto fail;
        }
    }
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        out->class = read_unsigned_bits(str_current,16);
        if(out->class!=1) {
            goto fail;
        }
    }
    if(parser_fail(stream_check(str_current,32))) {
        goto fail;
    }
    {
        out->ttl = read_unsigned_bits(str_current,32);
    }
    uint16_t dep_rlength;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        dep_rlength = read_unsigned_bits(str_current,16);
    }
    pos i4 = 0 ;
    out->rdata.count= dep_rlength;
    out->rdata.elem = n_malloc(arena,out->rdata.count*sizeof(out->rdata.elem[i4]));
    if(parser_fail(!out->rdata.elem)) {
        return -1;
    }
    for(; i4<out->rdata.count; i4++) {
        if(parser_fail(stream_check(str_current,8))) {
            goto fail;
        }
        {
            out->rdata.elem[i4] = read_unsigned_bits(str_current,8);
        }
    }
    return 0;
fail:
    return -1;
}
answer* parse_answer(NailArena *arena, NailStream *stream) {
    answer*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_answer(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_dnspacket(NailArena *arena,NailArena *tmparena,dnspacket *out,NailStream *str_current) {
    pos i;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        out->id = read_unsigned_bits(str_current,16);
    }
    if(parser_fail(stream_check(str_current,1))) {
        goto fail;
    }
    {
        out->qr = read_unsigned_bits(str_current,1);
    }
    if(parser_fail(stream_check(str_current,4))) {
        goto fail;
    }
    {
        out->opcode = read_unsigned_bits(str_current,4);
    }
    if(parser_fail(stream_check(str_current,1))) {
        goto fail;
    }
    {
        out->aa = read_unsigned_bits(str_current,1);
    }
    if(parser_fail(stream_check(str_current,1))) {
        goto fail;
    }
    {
        out->tc = read_unsigned_bits(str_current,1);
    }
    if(parser_fail(stream_check(str_current,1))) {
        goto fail;
    }
    {
        out->rd = read_unsigned_bits(str_current,1);
    }
    if(parser_fail(stream_check(str_current,1))) {
        goto fail;
    }
    {
        out->ra = read_unsigned_bits(str_current,1);
    }
    if(parser_fail(stream_check(str_current,3))) {
        goto fail;
    }
    if( read_unsigned_bits(str_current,3)!= 0) {
        goto fail;
    }
    if(parser_fail(stream_check(str_current,4))) {
        goto fail;
    }
    {
        out->rcode = read_unsigned_bits(str_current,4);
    }
    uint16_t dep_questioncount;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        dep_questioncount = read_unsigned_bits(str_current,16);
    }
    uint16_t dep_answercount;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        dep_answercount = read_unsigned_bits(str_current,16);
    }
    uint16_t dep_authoritycount;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        dep_authoritycount = read_unsigned_bits(str_current,16);
    }
    uint16_t dep_additionalcount;
    if(parser_fail(stream_check(str_current,16))) {
        goto fail;
    }
    {
        dep_additionalcount = read_unsigned_bits(str_current,16);
    }
    pos i5 = 0 ;
    out->questions.count= dep_questioncount;
    out->questions.elem = n_malloc(arena,out->questions.count*sizeof(out->questions.elem[i5]));
    if(parser_fail(!out->questions.elem)) {
        return -1;
    }
    for(; i5<out->questions.count; i5++) {
        if(parser_fail(peg_question(arena,tmparena, &out->questions.elem[i5],str_current ))) {
            goto fail;
        }
    }
    pos i6 = 0 ;
    out->responses.count= dep_answercount;
    out->responses.elem = n_malloc(arena,out->responses.count*sizeof(out->responses.elem[i6]));
    if(parser_fail(!out->responses.elem)) {
        return -1;
    }
    for(; i6<out->responses.count; i6++) {
        if(parser_fail(peg_answer(arena,tmparena, &out->responses.elem[i6],str_current ))) {
            goto fail;
        }
    }
    pos i7 = 0 ;
    out->authority.count= dep_authoritycount;
    out->authority.elem = n_malloc(arena,out->authority.count*sizeof(out->authority.elem[i7]));
    if(parser_fail(!out->authority.elem)) {
        return -1;
    }
    for(; i7<out->authority.count; i7++) {
        if(parser_fail(peg_answer(arena,tmparena, &out->authority.elem[i7],str_current ))) {
            goto fail;
        }
    }
    pos i8 = 0 ;
    out->additional.count= dep_additionalcount;
    out->additional.elem = n_malloc(arena,out->additional.count*sizeof(out->additional.elem[i8]));
    if(parser_fail(!out->additional.elem)) {
        return -1;
    }
    for(; i8<out->additional.count; i8++) {
        if(parser_fail(peg_answer(arena,tmparena, &out->additional.elem[i8],str_current ))) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
dnspacket* parse_dnspacket(NailArena *arena, NailStream *stream) {
    dnspacket*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_dnspacket(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_WHITE(NailStream *str_current) {
constmany_9_repeat:
    {   /*CUNION*/
        NailStreamPos back = stream_getpos(str_current);
        if(parser_fail(stream_check(str_current,8))) {
            goto cunion_11_1;
        }
        if( read_unsigned_bits(str_current,8)!= ' ') {
            goto cunion_11_1;
        }
        goto cunion_11_succ;
cunion_11_1:
        stream_reposition(str_current,back);
        if(parser_fail(stream_check(str_current,8))) {
            goto cunion_11_2;
        }
        if( read_unsigned_bits(str_current,8)!= '\n') {
            goto cunion_11_2;
        }
        goto cunion_11_succ;
cunion_11_2:
        stream_reposition(str_current,back);
        goto constmany_10_end;
cunion_11_succ:
        ;
    }
    goto constmany_9_repeat;
constmany_10_end:
    return 0;
fail:
    return -1;
}
static pos peg_number(NailArena *arena,NailArena *tmparena,number *out,NailStream *str_current) {
    pos i;
    {
        pos count_12= 0;
        struct {
            typeof(out->elem[count_12]) elem;
            void *prev;
        } *tmp_12 = NULL;
succ_repeat_12:
        ;
        NailStreamPos posrep_12= stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        typeof(tmp_12) prev_tmp_12= tmp_12;
        tmp_12 = n_malloc(tmparena,sizeof(*tmp_12));
        if(parser_fail(!tmp_12)) {
            return -1;
        }
        tmp_12->prev = prev_tmp_12;
        if(parser_fail(stream_check(str_current,8))) {
            goto fail_repeat_12;
        }
        {
            tmp_12[0].elem = read_unsigned_bits(str_current,8);
            if((tmp_12[0].elem>'9'||tmp_12[0].elem<'0')) {
                goto fail_repeat_12;
            }
        }
        count_12++;
        goto succ_repeat_12;
fail_repeat_12:
        tmp_12= tmp_12->prev;
fail_repeat_sep_12:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, posrep_12);
        out->count= count_12;
        out->elem= n_malloc(arena,sizeof(out->elem[count_12])*out->count);
        if(parser_fail(!out->elem)) {
            return -1;
        }
        while(count_12) {
            count_12--;
            memcpy(&out->elem[count_12],&tmp_12->elem,sizeof(out->elem[count_12]));
            tmp_12 = tmp_12->prev;
        }
    }
    return 0;
fail:
    return -1;
}
number* parse_number(NailArena *arena, NailStream *stream) {
    number*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_number(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_ipaddr(NailArena *arena,NailArena *tmparena,ipaddr *out,NailStream *str_current) {
    pos i;
    {
        pos count_13= 0;
        struct {
            typeof(out->elem[count_13]) elem;
            void *prev;
        } *tmp_13 = NULL;
        goto start_repeat_13;
succ_repeat_13:
        ;
        NailStreamPos posrep_13= stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        if(parser_fail(stream_check(str_current,8))) {
            goto fail_repeat_sep_13;
        }
        if( read_unsigned_bits(str_current,8)!= '.') {
            goto fail_repeat_sep_13;
        }
start_repeat_13:
        ;
        typeof(tmp_13) prev_tmp_13= tmp_13;
        tmp_13 = n_malloc(tmparena,sizeof(*tmp_13));
        if(parser_fail(!tmp_13)) {
            return -1;
        }
        tmp_13->prev = prev_tmp_13;
        if(parser_fail(peg_number(arena,tmparena, &tmp_13[0].elem,str_current ))) {
            goto fail_repeat_13;
        }
        count_13++;
        goto succ_repeat_13;
fail_repeat_13:
        tmp_13= tmp_13->prev;
fail_repeat_sep_13:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, posrep_13);
        if(count_13==0) {
            goto fail;
        }
        out->count= count_13;
        out->elem= n_malloc(arena,sizeof(out->elem[count_13])*out->count);
        if(parser_fail(!out->elem)) {
            return -1;
        }
        while(count_13) {
            count_13--;
            memcpy(&out->elem[count_13],&tmp_13->elem,sizeof(out->elem[count_13]));
            tmp_13 = tmp_13->prev;
        }
    }
    return 0;
fail:
    return -1;
}
ipaddr* parse_ipaddr(NailArena *arena, NailStream *stream) {
    ipaddr*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_ipaddr(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_alnum(NailArena *arena,NailArena *tmparena,alnum *out,NailStream *str_current) {
    pos i;
    if(parser_fail(stream_check(str_current,8))) {
        goto fail;
    }
    {
        out[0] = read_unsigned_bits(str_current,8);
        if((out[0]>'z'||out[0]<'a') && (out[0]>'9'||out[0]<'0')) {
            goto fail;
        }
    }
    return 0;
fail:
    return -1;
}
alnum* parse_alnum(NailArena *arena, NailStream *stream) {
    alnum*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_alnum(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_domain(NailArena *arena,NailArena *tmparena,domain *out,NailStream *str_current) {
    pos i;
    {
        pos count_14= 0;
        struct {
            typeof(out->elem[count_14]) elem;
            void *prev;
        } *tmp_14 = NULL;
        goto start_repeat_14;
succ_repeat_14:
        ;
        NailStreamPos posrep_14= stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        if(parser_fail(stream_check(str_current,8))) {
            goto fail_repeat_sep_14;
        }
        if( read_unsigned_bits(str_current,8)!= '.') {
            goto fail_repeat_sep_14;
        }
start_repeat_14:
        ;
        typeof(tmp_14) prev_tmp_14= tmp_14;
        tmp_14 = n_malloc(tmparena,sizeof(*tmp_14));
        if(parser_fail(!tmp_14)) {
            return -1;
        }
        tmp_14->prev = prev_tmp_14;
        {
            pos count_15= 0;
            struct {
                typeof(tmp_14[0].elem.elem[count_15]) elem;
                void *prev;
            } *tmp_15 = NULL;
succ_repeat_15:
            ;
            NailStreamPos posrep_15= stream_getpos(str_current);
            NailArenaPos back_arena = n_arena_save(arena);
            typeof(tmp_15) prev_tmp_15= tmp_15;
            tmp_15 = n_malloc(tmparena,sizeof(*tmp_15));
            if(parser_fail(!tmp_15)) {
                return -1;
            }
            tmp_15->prev = prev_tmp_15;
            if(parser_fail(peg_alnum(arena,tmparena, &tmp_15[0].elem,str_current ))) {
                goto fail_repeat_15;
            }
            count_15++;
            goto succ_repeat_15;
fail_repeat_15:
            tmp_15= tmp_15->prev;
fail_repeat_sep_15:
            n_arena_restore(arena, back_arena);
            stream_reposition(str_current, posrep_15);
            if(count_15==0) {
                goto fail_repeat_14;
            }
            tmp_14[0].elem.count= count_15;
            tmp_14[0].elem.elem= n_malloc(arena,sizeof(tmp_14[0].elem.elem[count_15])*tmp_14[0].elem.count);
            if(parser_fail(!tmp_14[0].elem.elem)) {
                return -1;
            }
            while(count_15) {
                count_15--;
                memcpy(&tmp_14[0].elem.elem[count_15],&tmp_15->elem,sizeof(tmp_14[0].elem.elem[count_15]));
                tmp_15 = tmp_15->prev;
            }
        }
        count_14++;
        goto succ_repeat_14;
fail_repeat_14:
        tmp_14= tmp_14->prev;
fail_repeat_sep_14:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, posrep_14);
        if(count_14==0) {
            goto fail;
        }
        out->count= count_14;
        out->elem= n_malloc(arena,sizeof(out->elem[count_14])*out->count);
        if(parser_fail(!out->elem)) {
            return -1;
        }
        while(count_14) {
            count_14--;
            memcpy(&out->elem[count_14],&tmp_14->elem,sizeof(out->elem[count_14]));
            tmp_14 = tmp_14->prev;
        }
    }
    return 0;
fail:
    return -1;
}
domain* parse_domain(NailArena *arena, NailStream *stream) {
    domain*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_domain(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_definition(NailArena *arena,NailArena *tmparena,definition *out,NailStream *str_current) {
    pos i;
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    if(parser_fail(peg_domain(arena,tmparena, &out->hostname,str_current ))) {
        goto fail;
    }
    {   /*CHOICE*/NailStreamPos back = stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_ns_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_ns_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_ns_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'N') {
            goto choice_16_ns_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_ns_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'S') {
            goto choice_16_ns_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_ns_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_ns_out;
        }
        if(parser_fail(peg_domain(arena,tmparena, &out->rr.ns,str_current ))) {
            goto choice_16_ns_out;
        }
        out->rr.N_type= NS;
        goto choice_16_succ;
choice_16_ns_out:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, back);
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'C') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'N') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'A') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'M') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'E') {
            goto choice_16_cname_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_cname_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_cname_out;
        }
        if(parser_fail(peg_domain(arena,tmparena, &out->rr.cname,str_current ))) {
            goto choice_16_cname_out;
        }
        out->rr.N_type= CNAME;
        goto choice_16_succ;
choice_16_cname_out:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, back);
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_mx_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_mx_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_mx_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'M') {
            goto choice_16_mx_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_mx_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'X') {
            goto choice_16_mx_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_mx_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_mx_out;
        }
        if(parser_fail(peg_domain(arena,tmparena, &out->rr.mx,str_current ))) {
            goto choice_16_mx_out;
        }
        out->rr.N_type= MX;
        goto choice_16_succ;
choice_16_mx_out:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, back);
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_a_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_a_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_a_out;
        }
        if( read_unsigned_bits(str_current,8)!= 'A') {
            goto choice_16_a_out;
        }
        if(parser_fail(stream_check(str_current,8))) {
            goto choice_16_a_out;
        }
        if( read_unsigned_bits(str_current,8)!= ':') {
            goto choice_16_a_out;
        }
        if(parser_fail(peg_domain(arena,tmparena, &out->rr.a,str_current ))) {
            goto choice_16_a_out;
        }
        out->rr.N_type= A;
        goto choice_16_succ;
choice_16_a_out:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, back);
        goto fail;
choice_16_succ:
        ;
    }
    return 0;
fail:
    return -1;
}
definition* parse_definition(NailArena *arena, NailStream *stream) {
    definition*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_definition(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
static pos peg_zone(NailArena *arena,NailArena *tmparena,zone *out,NailStream *str_current) {
    pos i;
    {
        pos count_17= 0;
        struct {
            typeof(out->elem[count_17]) elem;
            void *prev;
        } *tmp_17 = NULL;
succ_repeat_17:
        ;
        NailStreamPos posrep_17= stream_getpos(str_current);
        NailArenaPos back_arena = n_arena_save(arena);
        typeof(tmp_17) prev_tmp_17= tmp_17;
        tmp_17 = n_malloc(tmparena,sizeof(*tmp_17));
        if(parser_fail(!tmp_17)) {
            return -1;
        }
        tmp_17->prev = prev_tmp_17;
        if(parser_fail(peg_definition(arena,tmparena, &tmp_17[0].elem,str_current ))) {
            goto fail_repeat_17;
        }
        count_17++;
        goto succ_repeat_17;
fail_repeat_17:
        tmp_17= tmp_17->prev;
fail_repeat_sep_17:
        n_arena_restore(arena, back_arena);
        stream_reposition(str_current, posrep_17);
        if(count_17==0) {
            goto fail;
        }
        out->count= count_17;
        out->elem= n_malloc(arena,sizeof(out->elem[count_17])*out->count);
        if(parser_fail(!out->elem)) {
            return -1;
        }
        while(count_17) {
            count_17--;
            memcpy(&out->elem[count_17],&tmp_17->elem,sizeof(out->elem[count_17]));
            tmp_17 = tmp_17->prev;
        }
    }
    if(parser_fail(peg_WHITE(str_current))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
zone* parse_zone(NailArena *arena, NailStream *stream) {
    zone*retval = n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_zone(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream_check(stream,8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}

int gen_labels(NailArena *tmp_arena,NailStream *out,labels * val);
int gen_compressed(NailArena *tmp_arena,NailStream *out,compressed * val);
extern  int dnscompress_generate(NailArena *tmp_arena,NailStream *str_decompressed,NailStream *str_current);
int gen_question(NailArena *tmp_arena,NailStream *out,question * val);
int gen_answer(NailArena *tmp_arena,NailStream *out,answer * val);
int gen_dnspacket(NailArena *tmp_arena,NailStream *out,dnspacket * val);
int gen_WHITE(NailStream* str_current);
int gen_number(NailArena *tmp_arena,NailStream *out,number * val);
int gen_ipaddr(NailArena *tmp_arena,NailStream *out,ipaddr * val);
int gen_alnum(NailArena *tmp_arena,NailStream *out,alnum * val);
int gen_domain(NailArena *tmp_arena,NailStream *out,domain * val);
int gen_definition(NailArena *tmp_arena,NailStream *out,definition * val);
int gen_zone(NailArena *tmp_arena,NailStream *out,zone * val);
int gen_labels(NailArena *tmp_arena,NailStream *str_current,labels * val) {
    for(int i0=0; i0<val->count; i0++) {
        uint8_t dep_length;
        NailStreamPos rewind_length=stream_getpos(str_current);
        stream_output(str_current,0,8);
        for(int i1=0; i1<val->elem[i0].label.count; i1++) {
            if(parser_fail(stream_output(str_current,val->elem[i0].label.elem[i1],8))) return -1;
        }
        dep_length=val->elem[i0].label.count;
        {/*Context-rewind*/
            NailStreamPos  end_of_struct= stream_getpos(str_current);
            stream_reposition(str_current, rewind_length);
            stream_output(str_current,dep_length,8);
            stream_reposition(str_current, end_of_struct);
        }
    }
    if(parser_fail(stream_output(str_current,0,8))) return -1;
    return 0;
}
int gen_compressed(NailArena *tmp_arena,NailStream *str_current,compressed * val) {
    NailStream str_decompressed;
    if(parser_fail(NailOutStream_init(&str_decompressed,4096))) {
        return -1;
    }
    {   /*APPLY*/NailStream  * orig_str = str_current;
        str_current =&str_decompressed;
        if(parser_fail(gen_labels(tmp_arena,str_current,&val->labels))) {
            return -1;
        }
        str_current = orig_str;
    }
    if(parser_fail(dnscompress_generate(tmp_arena, &str_decompressed,str_current))) {
        return -1;
    }{/*Context-rewind*/
        NailStreamPos  end_of_struct= stream_getpos(str_current);
        NailOutStream_release(&str_decompressed);
        stream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_question(NailArena *tmp_arena,NailStream *str_current,question * val) {
    if(parser_fail(gen_compressed(tmp_arena,str_current,&val->labels))) {
        return -1;
    }
    if((val->qtype>16||val->qtype<1)) {
        return -1;
    }
    if(parser_fail(stream_output(str_current,val->qtype,16))) return -1;
    if(val->qclass!=1 && val->qclass!=255) {
        return -1;
    }
    if(parser_fail(stream_output(str_current,val->qclass,16))) return -1;
    {/*Context-rewind*/
        NailStreamPos  end_of_struct= stream_getpos(str_current);
        stream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_answer(NailArena *tmp_arena,NailStream *str_current,answer * val) {
    if(parser_fail(gen_compressed(tmp_arena,str_current,&val->labels))) {
        return -1;
    }
    if((val->rtype>16||val->rtype<1)) {
        return -1;
    }
    if(parser_fail(stream_output(str_current,val->rtype,16))) return -1;
    if(val->class!=1) {
        return -1;
    }
    if(parser_fail(stream_output(str_current,val->class,16))) return -1;
    if(parser_fail(stream_output(str_current,val->ttl,32))) return -1;
    uint16_t dep_rlength;
    NailStreamPos rewind_rlength=stream_getpos(str_current);
    stream_output(str_current,0,16);
    for(int i2=0; i2<val->rdata.count; i2++) {
        if(parser_fail(stream_output(str_current,val->rdata.elem[i2],8))) return -1;
    }
    dep_rlength=val->rdata.count;
    {/*Context-rewind*/
        NailStreamPos  end_of_struct= stream_getpos(str_current);
        stream_reposition(str_current, rewind_rlength);
        stream_output(str_current,dep_rlength,16);
        stream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_dnspacket(NailArena *tmp_arena,NailStream *str_current,dnspacket * val) {
    if(parser_fail(stream_output(str_current,val->id,16))) return -1;
    if(parser_fail(stream_output(str_current,val->qr,1))) return -1;
    if(parser_fail(stream_output(str_current,val->opcode,4))) return -1;
    if(parser_fail(stream_output(str_current,val->aa,1))) return -1;
    if(parser_fail(stream_output(str_current,val->tc,1))) return -1;
    if(parser_fail(stream_output(str_current,val->rd,1))) return -1;
    if(parser_fail(stream_output(str_current,val->ra,1))) return -1;
    if(parser_fail(stream_output(str_current,0,3))) return -1;
    if(parser_fail(stream_output(str_current,val->rcode,4))) return -1;
    uint16_t dep_questioncount;
    NailStreamPos rewind_questioncount=stream_getpos(str_current);
    stream_output(str_current,0,16);
    uint16_t dep_answercount;
    NailStreamPos rewind_answercount=stream_getpos(str_current);
    stream_output(str_current,0,16);
    uint16_t dep_authoritycount;
    NailStreamPos rewind_authoritycount=stream_getpos(str_current);
    stream_output(str_current,0,16);
    uint16_t dep_additionalcount;
    NailStreamPos rewind_additionalcount=stream_getpos(str_current);
    stream_output(str_current,0,16);
    for(int i3=0; i3<val->questions.count; i3++) {
        if(parser_fail(gen_question(tmp_arena,str_current,&val->questions.elem[i3]))) {
            return -1;
        }
    }
    dep_questioncount=val->questions.count;
    for(int i4=0; i4<val->responses.count; i4++) {
        if(parser_fail(gen_answer(tmp_arena,str_current,&val->responses.elem[i4]))) {
            return -1;
        }
    }
    dep_answercount=val->responses.count;
    for(int i5=0; i5<val->authority.count; i5++) {
        if(parser_fail(gen_answer(tmp_arena,str_current,&val->authority.elem[i5]))) {
            return -1;
        }
    }
    dep_authoritycount=val->authority.count;
    for(int i6=0; i6<val->additional.count; i6++) {
        if(parser_fail(gen_answer(tmp_arena,str_current,&val->additional.elem[i6]))) {
            return -1;
        }
    }
    dep_additionalcount=val->additional.count;
    {/*Context-rewind*/
        NailStreamPos  end_of_struct= stream_getpos(str_current);
        stream_reposition(str_current, rewind_questioncount);
        stream_output(str_current,dep_questioncount,16);
        stream_reposition(str_current, rewind_answercount);
        stream_output(str_current,dep_answercount,16);
        stream_reposition(str_current, rewind_authoritycount);
        stream_output(str_current,dep_authoritycount,16);
        stream_reposition(str_current, rewind_additionalcount);
        stream_output(str_current,dep_additionalcount,16);
        stream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_WHITE(NailStream* str_current) {
    if(parser_fail(stream_output(str_current,' ',8))) return -1;
    return 0;
}
int gen_number(NailArena *tmp_arena,NailStream *str_current,number * val) {
    for(int i7=0; i7<val->count; i7++) {
        if((val->elem[i7]>'9'||val->elem[i7]<'0')) {
            return -1;
        }
        if(parser_fail(stream_output(str_current,val->elem[i7],8))) return -1;
    }
    return 0;
}
int gen_ipaddr(NailArena *tmp_arena,NailStream *str_current,ipaddr * val) {
    for(int i8=0; i8<val->count; i8++) {
        if(i8!= 0) {
            if(parser_fail(stream_output(str_current,'.',8))) return -1;
        }
        if(parser_fail(gen_number(tmp_arena,str_current,&val->elem[i8]))) {
            return -1;
        }
    }
    return 0;
}
int gen_alnum(NailArena *tmp_arena,NailStream *str_current,alnum * val) {
    if((val[0]>'z'||val[0]<'a') && (val[0]>'9'||val[0]<'0')) {
        return -1;
    }
    if(parser_fail(stream_output(str_current,val[0],8))) return -1;
    return 0;
}
int gen_domain(NailArena *tmp_arena,NailStream *str_current,domain * val) {
    for(int i9=0; i9<val->count; i9++) {
        if(i9!= 0) {
            if(parser_fail(stream_output(str_current,'.',8))) return -1;
        }
        for(int i10=0; i10<val->elem[i9].count; i10++) {
            if(parser_fail(gen_alnum(tmp_arena,str_current,&val->elem[i9].elem[i10]))) {
                return -1;
            }
        }
    }
    return 0;
}
int gen_definition(NailArena *tmp_arena,NailStream *str_current,definition * val) {
    gen_WHITE(str_current);
    if(parser_fail(gen_domain(tmp_arena,str_current,&val->hostname))) {
        return -1;
    }
    switch(val->rr.N_type) {
    case NS:
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(stream_output(str_current,'N',8))) return -1;
        if(parser_fail(stream_output(str_current,'S',8))) return -1;
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(gen_domain(tmp_arena,str_current,&val->rr.ns))) {
            return -1;
        }
        break;
    case CNAME:
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(stream_output(str_current,'C',8))) return -1;
        if(parser_fail(stream_output(str_current,'N',8))) return -1;
        if(parser_fail(stream_output(str_current,'A',8))) return -1;
        if(parser_fail(stream_output(str_current,'M',8))) return -1;
        if(parser_fail(stream_output(str_current,'E',8))) return -1;
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(gen_domain(tmp_arena,str_current,&val->rr.cname))) {
            return -1;
        }
        break;
    case MX:
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(stream_output(str_current,'M',8))) return -1;
        if(parser_fail(stream_output(str_current,'X',8))) return -1;
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(gen_domain(tmp_arena,str_current,&val->rr.mx))) {
            return -1;
        }
        break;
    case A:
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(stream_output(str_current,'A',8))) return -1;
        if(parser_fail(stream_output(str_current,':',8))) return -1;
        if(parser_fail(gen_domain(tmp_arena,str_current,&val->rr.a))) {
            return -1;
        }
        break;
    }{/*Context-rewind*/
        NailStreamPos  end_of_struct= stream_getpos(str_current);
        stream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_zone(NailArena *tmp_arena,NailStream *str_current,zone * val) {
    for(int i11=0; i11<val->count; i11++) {
        if(parser_fail(gen_definition(tmp_arena,str_current,&val->elem[i11]))) {
            return -1;
        }
    }
    gen_WHITE(str_current);
    return 0;
}


