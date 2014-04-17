#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef int32_t pos;
typedef struct{
        pos *trace;
        pos capacity,iter,grow;
} n_trace; 
static uint64_t read_unsigned_bits(NailStream *stream, unsigned count){ 
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
                else{
                        //This can use a lot of performance love
//TODO: implement other endianesses
                        out_idx--;
                        retval |= ((data[pos] >> (7-bit_offset)) & 1) << out_idx;
                        count--;
                        bit_offset++;
                        if(bit_offset > 7){
                                bit_offset -= 8;
                                pos++;
                        }
                }
        }
        stream->pos = pos;
        stream->bit_offset = bit_offset;
    return retval;
}
static int stream_check(const NailStream *stream, unsigned count){
        return stream->size - (count>>3) - ((stream->bit_offset + count & 7)>>3) >= stream->pos;
}
static void stream_advance(NailStream *stream, unsigned count){
        
        stream->pos += count >> 3;
        stream->bit_offset += count &7;
        if(stream->bit_offset > 7){
                stream->pos++;
                stream->bit_offset-=8;
        }
}
static void stream_backup(NailStream *stream, unsigned count){
        stream->pos -= count >> 3;
        stream->bit_offset -= count & 7;
        if(stream->bit_offset < 0){
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
static NailStreamPos   stream_getpos(NailStream *stream){
        return stream->pos << 3 + stream->bit_offset; //TODO: Overflow potential!
}

int NailOutStream_new(NailStream *out,size_t siz){
        out->data = (const uint8_t *)malloc(siz);
        if(!out->data)
                return 0;
        out->pos = 0;
        out->bit_offset = 0;
        out->size = siz;
        return 1;
}
void NailOutStream_release(NailStream *out){
        free(out->data);
        out->data = NULL;
}
const uint8_t * NailOutStream_buffer(NailStream *str,size_t *siz){
        if(str->bit_offset)
                return NULL;
        *siz =  str->pos;
        return str->data;
}
//TODO: Perhaps use a separate structure for output streams?
static int stream_output(NailStream *stream,uint64_t data, size_t count){
        if(stream->pos + count>>3 + 1 >= stream->size){
                //TODO: parametrize stream growth
                stream->data = realloc((void *)stream->data,stream->size+4096);
                stream->size+= 4096;
                if(!stream->data)
                        return 0;
        }
        uint8_t *streamdata = (uint8_t *)stream->data;
        while(count>0){
                if(stream->bit_offset == 0 && (count & 7) == 0){
                        streamdata[stream->pos] = (data >> count );
                        count -= 8;
                        stream->pos++;
                }
                else{
                        streamdata[stream->pos] |= ((data >> count) & 1) << (7-stream->bit_offset);
                        count--;
                        stream->bit_offset++;
                        if(stream->bit_offset>7){
                                stream->pos++;
                                stream->bit_offset= 0;
                        }
                }
        }
}
//#define BITSLICE(x, off, len) read_unsigned_bits(x,off,len)
/* trace is a minimalistic representation of the AST. Many parsers add a count, choice parsers add
 * two pos parameters (which choice was taken and where in the trace it begins)
 * const parsers emit a new input position  
*/
typedef struct{
        pos position;
        pos parser;
        pos result;        
} n_hash;

typedef struct{
//        p_hash *memo;
        unsigned lg_size; // How large is the hashtable - make it a power of two 
} n_hashtable;

static int  n_trace_init(n_trace *out,pos size,pos grow){
        if(size <= 1){
                return 0;
        }
        out->trace = (pos *)malloc(size * sizeof(pos));
        if(!out){
                return 0;
        }
        out->capacity = size -1 ;
        out->iter = 0;
        if(grow < 16){ // Grow needs to be at least 2, but small grow makes no sense
                grow = 16; 
        }
        out->grow = grow;
        return 1;
}
static void n_trace_release(n_trace *out){
        free(out->trace);
        out->trace =NULL;
        out->capacity = 0;
        out->iter = 0;
        out->grow = 0;
}
static pos n_trace_getpos(n_trace *tr){
        return tr->iter;
}
static void n_tr_setpos(n_trace *tr,pos offset){
        assert(offset<tr->capacity);
        tr->iter = offset;
}
static int n_trace_grow(n_trace *out, int space){
        if(out->capacity - space>= out->iter){
                return 0;
        }

        pos * new_ptr= (pos *)realloc(out->trace, (out->capacity + out->grow) * sizeof(pos));
        if(!new_ptr){
                return 1;
        }
        out->trace = new_ptr;
        out->capacity += out->grow;
        return 0;
}
static pos n_tr_memo_optional(n_trace *trace){
        if(n_trace_grow(trace,1))
                return -1;
        trace->trace[trace->iter] = 0xFFFFFFFD;
        return trace->iter++;
}
static void n_tr_optional_succeed(n_trace * trace, pos where){
        trace->trace[where] = -1;
}
static void n_tr_optional_fail(n_trace * trace, pos where){
        trace->trace[where] = trace->iter;
}
static pos n_tr_memo_many(n_trace *trace){
        if(n_trace_grow(trace,2))
                return -1;
        trace->trace[trace->iter] = 0xFFFFFFFE;
        trace->trace[trace->iter+1] = 0xEEEEEEEF;
        trace->iter +=2;
        return trace->iter-2;

}
static void n_tr_write_many(n_trace *trace, pos where, pos count){
        trace->trace[where] = count;
        trace->trace[where+1] = trace->iter;
        fprintf(stderr,"%d = many %d %d\n", where, count,trace->iter);
}

static pos n_tr_begin_choice(n_trace *trace){
        if(n_trace_grow(trace,2))
                return -1;
      
        //Debugging values
        trace->trace[trace->iter] = 0xFFFFFFFF;
        trace->trace[trace->iter+1] = 0xEEEEEEEE;
        trace->iter+= 2;
        return trace->iter - 2;
}
static int n_tr_stream(n_trace *trace, const NailStream *stream){
        assert(sizeof(stream) % sizeof(pos) == 0);
        if(n_trace_grow(trace,sizeof(*stream)/sizeof(pos)))
                return -1;
        *(NailStream *)(trace->trace + trace->iter) = *stream;
        fprintf(stderr,"%d = stream\n",trace->iter,stream);
        trace->iter += sizeof(*stream)/sizeof(pos);  
        return 0;
}
static pos n_tr_memo_choice(n_trace *trace){
        return trace->iter;
}
static void n_tr_pick_choice(n_trace *trace, pos where, pos which_choice, pos  choice_begin){
        trace->trace[where] = which_choice;
        trace->trace[where + 1] = choice_begin;
        fprintf(stderr,"%d = pick %d %d\n",where, which_choice,choice_begin);
}
static int n_tr_const(n_trace *trace,NailStream *stream){
        if(n_trace_grow(trace,1))
                return -1;
        NailStreamPos newoff = stream_getpos(stream);
        fprintf(stderr,"%d = const %d \n",trace->iter, newoff);        
        trace->trace[trace->iter++] = newoff;
        return 0;
}
#define n_tr_offset n_tr_const
typedef struct NailArenaPool{
        char *iter;char *end;
        struct NailArenaPool *next;
} NailArenaPool;

void *n_malloc(NailArena *arena, size_t size)
{
        void *retval;
        if(arena->current->end - arena->current->iter <= size){
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

int NailArena_init(NailArena *arena, size_t blocksize){
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
int NailArena_release(NailArena *arena){
        NailArenaPool *p;
        while((p= arena->current) ){
                arena->current = p->next;
                free(p);
        }
        arena->blocksize = 0;
        return 0;
}
//Returns the pointer where the taken choice is supposed to go.

#define parser_fail(i) __builtin_expect(i<0,0)



