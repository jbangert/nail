#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define parser_fail(i) __builtin_expect((i)!=0,0)

static int stream_reposition(NailStream *stream, NailStreamPos p)
{
    stream->pos = p >> 3;
    stream->bit_offset = p & 7;
    return 0;
}
static int NailOutStream_reposition(NailOutStream *stream, NailOutStreamPos p)
{
    stream->pos = p >> 3;
    stream->bit_offset = p & 7;
    return 0;
}
static NailStreamPos   stream_getpos(NailStream *stream) {
    return (stream->pos << 3) + stream->bit_offset; //TODO: Overflow potential!
}
static NailOutStreamPos   NailOutStream_getpos(NailOutStream *stream) {
    return (stream->pos << 3) + stream->bit_offset; //TODO: Overflow potential!
}


int NailOutStream_init(NailOutStream *out,size_t siz) {
    out->data = ( uint8_t *)malloc(siz);
    if(!out->data)
        return -1;
    out->pos = 0;
    out->bit_offset = 0;
    out->size = siz;
    return 0;
}
void NailOutStream_release(NailOutStream *out) {
    free((void *)out->data);
    out->data = NULL;
}
const uint8_t * NailOutStream_buffer(NailOutStream *str,size_t *siz) {
    if(str->bit_offset)
        return NULL;
    *siz =  str->pos;
    return str->data;
}
//TODO: Perhaps use a separate structure for output streams?
int NailOutStream_grow(NailOutStream *stream, size_t count) {
    if(stream->pos + (count>>3) + 1 >= stream->size) {
        //TODO: parametrize stream growth
        int alloc_size = stream->pos + (count>>3) + 1;
        if(4096+stream->size>alloc_size) alloc_size = 4096+stream->size;
        stream->data = (uint8_t *)realloc((void *)stream->data,alloc_size);
        stream->size = alloc_size;
        if(!stream->data)
            return -1;
    }
    return 0;
}
static int NailOutStream_write(NailOutStream *stream,uint64_t data, size_t count) {
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

NailArenaPos n_arena_save(NailArena *arena)
{
    NailArenaPos retval = {.pool = arena->current, .iter = arena->current->iter};
    return retval;
}
void n_arena_restore(NailArena *arena, NailArenaPos p) {
    arena->current = p.pool;
    arena->current->iter = p.iter;
    //memory will remain linked
}
//Returns the pointer where the taken choice is supposed to go.








template <typename strt_current> static int32_t peg_ip6contents(NailArena *arena,NailArena *tmparena,ip6contents *out,strt_current *str_current,uint8_t* dep_type);
template <typename strt_current> static int32_t peg_ip6addr(NailArena *arena,NailArena *tmparena,ip6addr *out,strt_current *str_current);
template <typename strt_current> static int32_t peg_ip6packet(NailArena *arena,NailArena *tmparena,ip6packet *out,strt_current *str_current);
template <typename strt_current> static int32_t peg_ethernet(NailArena *arena,NailArena *tmparena,ethernet *out,strt_current *str_current);
template <typename strt_current> static int32_t peg_foo(NailArena *arena,NailArena *tmparena,foo *out,strt_current *str_current);
template <typename strt_current> static int32_t peg_ip6contents(NailArena *arena,NailArena *tmparena,ip6contents *out,strt_current *str_current,uint8_t* dep_type) {
    int32_t i;
    switch(*dep_type) {
    case 0: {
        out->N_type= HOPBYHOP;
        break;
    }
    case 1: {
        out->N_type= ICMP;
        break;
    }
    case 17: {
        out->N_type= UDP;
        if(parser_fail(str_current->check(16))) {
            goto fail;
        }
        {
            out->udp.source = str_current->read_unsigned_big(16);
        }
        if(parser_fail(str_current->check(16))) {
            goto fail;
        }
        {
            out->udp.destination = str_current->read_unsigned_big(16);
        }
        uint16_t dep_length;
        if(parser_fail(str_current->check(16))) {
            goto fail;
        }
        {
            dep_length = str_current->read_unsigned_big(16);
        }
        if(parser_fail(str_current->check(16))) {
            goto fail;
        }
        {
            out->udp.checksum = str_current->read_unsigned_big(16);
        }
        ;
        typename total_size_parse<strt_current>::out_1_t *str_content;
        if(total_size_parse<strt_current>::f(tmparena, &str_content,str_current,&dep_length)) {
            goto fail;
        }{/*APPLY*/typeof(str_content) str_current = str_content;
            {
                int32_t count_2= 0;
                struct {
                    typeof(out->udp.payload.elem[count_2]) elem;
                    void *prev;
                } *tmp_2 = NULL;
                NailArenaPos back_arena = n_arena_save(arena);
                typeof(tmp_2) prev_tmp_2;
                typename strt_current::pos_t posrep_2= str_current->getpos();
succ_repeat_2:
                ;
                posrep_2= str_current->getpos();
                back_arena = n_arena_save(arena);
                prev_tmp_2= tmp_2;
                tmp_2 = (typeof(tmp_2))n_malloc(tmparena,sizeof(*tmp_2));
                if(parser_fail(!tmp_2)) {
                    return -1;
                }
                tmp_2->prev = prev_tmp_2;
                if(parser_fail(str_current->check(8))) {
                    goto fail_repeat_2;
                }
                {
                    tmp_2[0].elem = str_current->read_unsigned_big(8);
                }
                count_2++;
                goto succ_repeat_2;
fail_repeat_2:
                tmp_2= (typeof(tmp_2))tmp_2->prev;
fail_repeat_sep_2:
                n_arena_restore(arena, back_arena);
                str_current->rewind(posrep_2);
                out->udp.payload.count= count_2;
                out->udp.payload.elem= (typeof(out->udp.payload.elem))n_malloc(arena,sizeof(out->udp.payload.elem[count_2])*out->udp.payload.count);
                if(parser_fail(!out->udp.payload.elem)) {
                    return -1;
                }
                while(count_2) {
                    count_2--;
                    memcpy(&out->udp.payload.elem[count_2],&tmp_2->elem,sizeof(out->udp.payload.elem[count_2]));
                    tmp_2 = (typeof(tmp_2))tmp_2->prev;
                }
            }
            goto succ_apply_1;
fail_apply_1:
            goto fail;
succ_apply_1:
            ;
        }
        break;
    }
    case 43: {
        out->N_type= ROUTING;
        if(parser_fail(str_current->check(8))) {
            goto fail;
        }
        if( str_current->read_unsigned_big(8)!= 'F') {
            goto fail;
        }
        if(parser_fail(str_current->check(8))) {
            goto fail;
        }
        if( str_current->read_unsigned_big(8)!= 'A') {
            goto fail;
        }
        if(parser_fail(str_current->check(8))) {
            goto fail;
        }
        if( str_current->read_unsigned_big(8)!= 'I') {
            goto fail;
        }
        if(parser_fail(str_current->check(8))) {
            goto fail;
        }
        if( str_current->read_unsigned_big(8)!= 'L') {
            goto fail;
        }
        break;
    }
    case 44: {
        out->N_type= FRAGMENT;
        break;
    }
    default:
        goto fail;
        break;
    }
    return 0;
fail:
    return -1;
}
template <typename strt_current> static int32_t peg_ip6addr(NailArena *arena,NailArena *tmparena,ip6addr *out,strt_current *str_current) {
    int32_t i;
    if(parser_fail(str_current->check(64))) {
        goto fail;
    }
    {
        out->net = str_current->read_unsigned_big(64);
    }
    if(parser_fail(str_current->check(64))) {
        goto fail;
    }
    {
        out->host = str_current->read_unsigned_big(64);
    }
    return 0;
fail:
    return -1;
}
template <typename stream_t> ip6addr* parse_ip6addr(NailArena *arena, stream_t *stream) {
    ip6addr*retval = (ip6addr*)n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_ip6addr(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream->check(8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
template <typename strt_current> static int32_t peg_ip6packet(NailArena *arena,NailArena *tmparena,ip6packet *out,strt_current *str_current) {
    int32_t i;
    if(parser_fail(str_current->check(4))) {
        goto fail;
    }
    if( str_current->read_unsigned_big(4)!= 6) {
        goto fail;
    }
    if(parser_fail(str_current->check(6))) {
        goto fail;
    }
    {
        out->diffserv = str_current->read_unsigned_big(6);
    }
    if(parser_fail(str_current->check(2))) {
        goto fail;
    }
    {
        out->ecn = str_current->read_unsigned_big(2);
    }
    if(parser_fail(str_current->check(20))) {
        goto fail;
    }
    {
        out->flow = str_current->read_unsigned_big(20);
    }
    uint16_t dep_payload_len;
    if(parser_fail(str_current->check(16))) {
        goto fail;
    }
    {
        dep_payload_len = str_current->read_unsigned_big(16);
    }
    uint8_t dep_next_header;
    if(parser_fail(str_current->check(8))) {
        goto fail;
    }
    {
        dep_next_header = str_current->read_unsigned_big(8);
    }
    if(parser_fail(str_current->check(8))) {
        goto fail;
    }
    {
        out->hops = str_current->read_unsigned_big(8);
    }
    if(parser_fail(peg_ip6addr(arena,tmparena, &out->saddr,str_current ))) {
        goto fail;
    }
    if(parser_fail(peg_ip6addr(arena,tmparena, &out->daddr,str_current ))) {
        goto fail;
    }
    ;
    typename size_parse<strt_current>::out_1_t *str_payload;
    if(size_parse<strt_current>::f(tmparena, &str_payload,str_current,&dep_payload_len)) {
        goto fail;
    }{/*APPLY*/typeof(str_payload) str_current = str_payload;
        if(parser_fail(peg_ip6contents(arena,tmparena, &out->payload,str_current ,&dep_next_header))) {
            goto fail_apply_3;
        }
        goto succ_apply_3;
fail_apply_3:
        goto fail;
succ_apply_3:
        ;
    }
    return 0;
fail:
    return -1;
}
template <typename stream_t> ip6packet* parse_ip6packet(NailArena *arena, stream_t *stream) {
    ip6packet*retval = (ip6packet*)n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_ip6packet(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream->check(8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
template <typename strt_current> static int32_t peg_ethernet(NailArena *arena,NailArena *tmparena,ethernet *out,strt_current *str_current) {
    int32_t i;
    if(parser_fail(str_current->check(48))) {
        goto fail;
    }
    {
        out->dest = str_current->read_unsigned_big(48);
    }
    if(parser_fail(str_current->check(48))) {
        goto fail;
    }
    {
        out->src = str_current->read_unsigned_big(48);
    }
    if(parser_fail(str_current->check(16))) {
        goto fail;
    }
    {
        out->ethertype = str_current->read_unsigned_big(16);
    }
    if(parser_fail(peg_ip6packet(arena,tmparena, &out->ip,str_current ))) {
        goto fail;
    }
    return 0;
fail:
    return -1;
}
template <typename stream_t> ethernet* parse_ethernet(NailArena *arena, stream_t *stream) {
    ethernet*retval = (ethernet*)n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_ethernet(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream->check(8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}
template <typename strt_current> static int32_t peg_foo(NailArena *arena,NailArena *tmparena,foo *out,strt_current *str_current) {
    int32_t i;
    if(parser_fail(str_current->check(32))) {
        goto fail;
    }
    if( str_current->read_unsigned_little(32)!= 0xa1b2c3d4) {
        goto fail;
    }
    if(parser_fail(str_current->check(16))) {
        goto fail;
    }
    {
        out->major = str_current->read_unsigned_little(16);
    }
    if(parser_fail(str_current->check(16))) {
        goto fail;
    }
    {
        out->minor = str_current->read_unsigned_little(16);
    }
    if(parser_fail(str_current->check(32))) {
        goto fail;
    }
    {
        out->zone = str_current->read_unsigned_little(32);
    }
    if(parser_fail(str_current->check(32))) {
        goto fail;
    }
    {
        out->figs = str_current->read_unsigned_little(32);
    }
    if(parser_fail(str_current->check(32))) {
        goto fail;
    }
    {
        out->snaplen = str_current->read_unsigned_little(32);
    }
    if(parser_fail(str_current->check(32))) {
        goto fail;
    }
    {
        out->network = str_current->read_unsigned_little(32);
    }
    {
        int32_t count_4= 0;
        struct {
            typeof(out->packets.elem[count_4]) elem;
            void *prev;
        } *tmp_4 = NULL;
        NailArenaPos back_arena = n_arena_save(arena);
        typeof(tmp_4) prev_tmp_4;
        typename strt_current::pos_t posrep_4= str_current->getpos();
succ_repeat_4:
        ;
        posrep_4= str_current->getpos();
        back_arena = n_arena_save(arena);
        prev_tmp_4= tmp_4;
        tmp_4 = (typeof(tmp_4))n_malloc(tmparena,sizeof(*tmp_4));
        if(parser_fail(!tmp_4)) {
            return -1;
        }
        tmp_4->prev = prev_tmp_4;
        if(parser_fail(str_current->check(32))) {
            goto fail_repeat_4;
        }
        {
            tmp_4[0].elem.sec = str_current->read_unsigned_little(32);
        }
        if(parser_fail(str_current->check(32))) {
            goto fail_repeat_4;
        }
        {
            tmp_4[0].elem.usec = str_current->read_unsigned_little(32);
        }
        uint32_t dep_length;
        if(parser_fail(str_current->check(32))) {
            goto fail_repeat_4;
        }
        {
            dep_length = str_current->read_unsigned_little(32);
        }
        uint32_t dep_orig_len;
        if(parser_fail(str_current->check(32))) {
            goto fail_repeat_4;
        }
        {
            dep_orig_len = str_current->read_unsigned_little(32);
        }
        ;
        typename size_parse<strt_current>::out_1_t *str_pstream;
        if(size_parse<strt_current>::f(tmparena, &str_pstream,str_current,&dep_length)) {
            goto fail_repeat_4;
        }{/*APPLY*/typeof(str_pstream) str_current = str_pstream;
            if(parser_fail(peg_ethernet(arena,tmparena, &tmp_4[0].elem.packet,str_current ))) {
                goto fail_apply_5;
            }
            goto succ_apply_5;
fail_apply_5:
            goto fail_repeat_4;
succ_apply_5:
            ;
        }
        count_4++;
        goto succ_repeat_4;
fail_repeat_4:
        tmp_4= (typeof(tmp_4))tmp_4->prev;
fail_repeat_sep_4:
        n_arena_restore(arena, back_arena);
        str_current->rewind(posrep_4);
        out->packets.count= count_4;
        out->packets.elem= (typeof(out->packets.elem))n_malloc(arena,sizeof(out->packets.elem[count_4])*out->packets.count);
        if(parser_fail(!out->packets.elem)) {
            return -1;
        }
        while(count_4) {
            count_4--;
            memcpy(&out->packets.elem[count_4],&tmp_4->elem,sizeof(out->packets.elem[count_4]));
            tmp_4 = (typeof(tmp_4))tmp_4->prev;
        }
    }
    return 0;
fail:
    return -1;
}
template <typename stream_t> foo* parse_foo(NailArena *arena, stream_t *stream) {
    foo*retval = (foo*)n_malloc(arena, sizeof(*retval));
    NailArena tmparena;
    NailArena_init(&tmparena, 4096);
    if(!retval) return NULL;
    if(parser_fail(peg_foo(arena, &tmparena,retval, stream))) {
        goto fail;
    }
    if(!stream->check(8)) {
        goto fail;
    }
    NailArena_release(&tmparena);
    return retval;
fail:
    NailArena_release(&tmparena);
    return NULL;
}

int gen_ip6contents(NailArena *tmp_arena,NailOutStream *out,ip6contents * val,uint8_t* dep_type);
int gen_ip6addr(NailArena *tmp_arena,NailOutStream *out,ip6addr * val);
int gen_ip6packet(NailArena *tmp_arena,NailOutStream *out,ip6packet * val);
int gen_ethernet(NailArena *tmp_arena,NailOutStream *out,ethernet * val);
int gen_foo(NailArena *tmp_arena,NailOutStream *out,foo * val);
int gen_ip6contents(NailArena *tmp_arena,NailOutStream *str_current,ip6contents * val,uint8_t* dep_type) {
    switch(val->N_type) {
    case HOPBYHOP: {
        *dep_type=0;
        {/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, end_of_struct);
        }
        break;
    }
    case ICMP: {
        *dep_type=1;
        {/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, end_of_struct);
        }
        break;
    }
    case UDP: {
        *dep_type=17;
        if(parser_fail(NailOutStream_write(str_current,val->udp.source,16))) return -1;
        if(parser_fail(NailOutStream_write(str_current,val->udp.destination,16))) return -1;
        uint16_t dep_length;
        NailOutStreamPos rewind_length=NailOutStream_getpos(str_current);
        NailOutStream_write(str_current,0,16);
        if(parser_fail(NailOutStream_write(str_current,val->udp.checksum,16))) return -1;
        NailOutStream str_content;
        if(parser_fail(NailOutStream_init(&str_content,4096))) {
            return -1;
        }
        {   /*APPLY*/NailOutStream  * orig_str = str_current;
            str_current =&str_content;
            for(int i0=0; i0<val->udp.payload.count; i0++) {
                if(parser_fail(NailOutStream_write(str_current,val->udp.payload.elem[i0],8))) return -1;
            }
            str_current = orig_str;
        }
        if(parser_fail(total_size_generate(tmp_arena, &str_content,str_current,&dep_length))) {
            return -1;
        }{/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, rewind_length);
            NailOutStream_write(str_current,dep_length,16);
            NailOutStream_release(&str_content);
            NailOutStream_reposition(str_current, end_of_struct);
        }
        break;
    }
    case ROUTING: {
        *dep_type=43;
        if(parser_fail(NailOutStream_write(str_current,'F',8))) return -1;
        if(parser_fail(NailOutStream_write(str_current,'A',8))) return -1;
        if(parser_fail(NailOutStream_write(str_current,'I',8))) return -1;
        if(parser_fail(NailOutStream_write(str_current,'L',8))) return -1;
        {/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, end_of_struct);
        }
        break;
    }
    case FRAGMENT: {
        *dep_type=44;
        {/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, end_of_struct);
        }
        break;
    }
    }
    return 0;
}
int gen_ip6addr(NailArena *tmp_arena,NailOutStream *str_current,ip6addr * val) {
    if(parser_fail(NailOutStream_write(str_current,val->net,64))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->host,64))) return -1;
    {/*Context-rewind*/
        NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
        NailOutStream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_ip6packet(NailArena *tmp_arena,NailOutStream *str_current,ip6packet * val) {
    if(parser_fail(NailOutStream_write(str_current,6,4))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->diffserv,6))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->ecn,2))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->flow,20))) return -1;
    uint16_t dep_payload_len;
    NailOutStreamPos rewind_payload_len=NailOutStream_getpos(str_current);
    NailOutStream_write(str_current,0,16);
    uint8_t dep_next_header;
    NailOutStreamPos rewind_next_header=NailOutStream_getpos(str_current);
    NailOutStream_write(str_current,0,8);
    if(parser_fail(NailOutStream_write(str_current,val->hops,8))) return -1;
    if(parser_fail(gen_ip6addr(tmp_arena,str_current,&val->saddr))) {
        return -1;
    }
    if(parser_fail(gen_ip6addr(tmp_arena,str_current,&val->daddr))) {
        return -1;
    }
    NailOutStream str_payload;
    if(parser_fail(NailOutStream_init(&str_payload,4096))) {
        return -1;
    }
    {   /*APPLY*/NailOutStream  * orig_str = str_current;
        str_current =&str_payload;
        if(parser_fail(gen_ip6contents(tmp_arena,str_current,&val->payload,&dep_next_header))) {
            return -1;
        }
        str_current = orig_str;
    }
    if(parser_fail(size_generate(tmp_arena, &str_payload,str_current,&dep_payload_len))) {
        return -1;
    }{/*Context-rewind*/
        NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
        NailOutStream_reposition(str_current, rewind_payload_len);
        NailOutStream_write(str_current,dep_payload_len,16);
        NailOutStream_reposition(str_current, rewind_next_header);
        NailOutStream_write(str_current,dep_next_header,8);
        NailOutStream_release(&str_payload);
        NailOutStream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_ethernet(NailArena *tmp_arena,NailOutStream *str_current,ethernet * val) {
    if(parser_fail(NailOutStream_write(str_current,val->dest,48))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->src,48))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->ethertype,16))) return -1;
    if(parser_fail(gen_ip6packet(tmp_arena,str_current,&val->ip))) {
        return -1;
    }{/*Context-rewind*/
        NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
        NailOutStream_reposition(str_current, end_of_struct);
    }
    return 0;
}
int gen_foo(NailArena *tmp_arena,NailOutStream *str_current,foo * val) {
    if(parser_fail(NailOutStream_write(str_current,0xa1b2c3d4,32))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->major,16))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->minor,16))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->zone,32))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->figs,32))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->snaplen,32))) return -1;
    if(parser_fail(NailOutStream_write(str_current,val->network,32))) return -1;
    for(int i1=0; i1<val->packets.count; i1++) {
        if(parser_fail(NailOutStream_write(str_current,val->packets.elem[i1].sec,32))) return -1;
        if(parser_fail(NailOutStream_write(str_current,val->packets.elem[i1].usec,32))) return -1;
        uint32_t dep_length;
        NailOutStreamPos rewind_length=NailOutStream_getpos(str_current);
        NailOutStream_write(str_current,0,32);
        uint32_t dep_orig_len;
        NailOutStreamPos rewind_orig_len=NailOutStream_getpos(str_current);
        NailOutStream_write(str_current,0,32);
        NailOutStream str_pstream;
        if(parser_fail(NailOutStream_init(&str_pstream,4096))) {
            return -1;
        }
        {   /*APPLY*/NailOutStream  * orig_str = str_current;
            str_current =&str_pstream;
            if(parser_fail(gen_ethernet(tmp_arena,str_current,&val->packets.elem[i1].packet))) {
                return -1;
            }
            str_current = orig_str;
        }
        if(parser_fail(size_generate(tmp_arena, &str_pstream,str_current,&dep_length))) {
            return -1;
        }{/*Context-rewind*/
            NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
            NailOutStream_reposition(str_current, rewind_length);
            NailOutStream_write(str_current,dep_length,32);
            NailOutStream_reposition(str_current, rewind_orig_len);
            NailOutStream_write(str_current,dep_orig_len,32);
            NailOutStream_release(&str_pstream);
            NailOutStream_reposition(str_current, end_of_struct);
        }
    }{/*Context-rewind*/
        NailOutStreamPos  end_of_struct= NailOutStream_getpos(str_current);
        NailOutStream_reposition(str_current, end_of_struct);
    }
    return 0;
}


