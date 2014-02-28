#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
typedef int32_t pos;
typedef struct{
        pos *trace;
        pos capacity,iter,grow;
} n_trace; 
uint64_t read_unsigned_bits(const char *data, pos pos, unsigned count){    uint64_t retval = 0;
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
        else{
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
        out->trace = malloc(size * sizeof(pos));
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
static int n_trace_grow(n_trace *out, int space){
        if(out->capacity - space>= out->iter){
                return 0;
        }

        pos * new_ptr= realloc(out->trace, out->capacity + out->grow);
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
static pos n_tr_memo_choice(n_trace *trace){
        return trace->iter;
}
static void n_tr_pick_choice(n_trace *trace, pos where, pos which_choice, pos  choice_begin){
        trace->trace[where] = which_choice;
        trace->trace[where + 1] = choice_begin;
        fprintf(stderr,"%d = pick %d %d\n",where, which_choice,choice_begin);
}
static int n_tr_const(n_trace *trace,pos newoff){
        if(n_trace_grow(trace,1))
                        return -1;
        fprintf(stderr,"%d = const %d \n",trace->iter, newoff);        
        trace->trace[trace->iter++] = newoff;
        return 0;
}

typedef struct NailArena_{} NailArena ;
//Returns the pointer where the taken choice is supposed to go.

#define parser_fail(i) __builtin_expect(i<0,0)



