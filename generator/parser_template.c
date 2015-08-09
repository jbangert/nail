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
#define parser_fail(i) __builtin_expect((i)<0,0)
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
        if(stream->size - (count>>3) - ((stream->bit_offset + count & 7)>>3) < stream->pos)
                return -1;
        return 0;
}
static void stream_advance(NailStream *stream, unsigned count){
        
        stream->pos += count >> 3;
        stream->bit_offset += count &7;
        if(stream->bit_offset > 7){
                stream->pos++;
                stream->bit_offset-=8;
        }
}
static NailStreamPos   stream_getpos(NailStream *stream){
  return (stream->pos << 3) + stream->bit_offset; //TODO: Overflow potential!
}
static void stream_backup(NailStream *stream, unsigned count){
        stream->pos -= count >> 3;
        stream->bit_offset -= count & 7;
        if(stream->bit_offset < 0){
                stream->pos--;
                stream->bit_offset += 8;
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
                return -1;
        }
        out->trace = (pos *)malloc(size * sizeof(pos));
        if(!out){
                return -1;
        }
        out->capacity = size -1 ;
        out->iter = 0;
        if(grow < 16){ // Grow needs to be at least 2, but small grow makes no sense
                grow = 16; 
        }
        out->grow = grow;
        return 0;
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
                return -1;
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
        if(parser_fail(n_trace_grow(trace,2)))
                return -1;
        trace->trace[trace->iter] = 0xFFFFFFFE;
        trace->trace[trace->iter+1] = 0xEEEEEEEF;
        trace->iter +=2;
        return trace->iter-2;

}
static void n_tr_write_many(n_trace *trace, pos where, pos count){
        trace->trace[where] = count;
        trace->trace[where+1] = trace->iter;
#ifdef NAIL_DEBUG
        fprintf(stderr,"%d = many %d %d\n", where, count,trace->iter);
#endif
}

static pos n_tr_begin_choice(n_trace *trace){
        if(parser_fail(n_trace_grow(trace,2)))
                return -1;
      
        //Debugging values
        trace->trace[trace->iter] = 0xFFFFFFFF;
        trace->trace[trace->iter+1] = 0xEEEEEEEE;
        trace->iter+= 2;
        return trace->iter - 2;
}
static int n_tr_stream(n_trace *trace, const NailStream *stream){
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
static pos n_tr_memo_choice(n_trace *trace){
        return trace->iter;
}
static void n_tr_pick_choice(n_trace *trace, pos where, pos which_choice, pos  choice_begin){
        trace->trace[where] = which_choice;
        trace->trace[where + 1] = choice_begin;
#ifdef NAIL_DEBUG
        fprintf(stderr,"%d = pick %d %d\n",where, which_choice,choice_begin);
#endif
}
static int n_tr_const(n_trace *trace,NailStream *stream){
        if(parser_fail(n_trace_grow(trace,1)))
                return -1;
        NailStreamPos newoff = stream_getpos(stream);
#ifdef NAIL_DEBUG
        fprintf(stderr,"%d = const %d \n",trace->iter, newoff);        
#endif
        trace->trace[trace->iter++] = newoff;
        return 0;
}
#define n_tr_offset n_tr_const
