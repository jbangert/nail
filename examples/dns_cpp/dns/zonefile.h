#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
typedef int32_t pos;
typedef struct{
        pos *trace;
        pos capacity,iter,grow;
} n_trace; 
uint64_t read_unsigned_bits(const uint8_t *data, pos pos, unsigned count){ 
        uint64_t retval = 0;
        unsigned int out_idx=count;
        //TODO: Implement little endian too
        //Count LSB to MSB
        while(count>0) {
                if((pos & 7) == 0 && (count &7) ==0) {
                        out_idx-=8;
                        retval|= data[pos >> 3] << out_idx;
                        pos += 8;
                        count-=8;
                }
                else{
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
static void n_trace_release(n_trace *out){
        free(out->trace);
        out->trace =NULL;
        out->capacity = 0;
        out->iter = 0;
        out->grow = 0;
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

typedef struct NailArenaPool{
        void *iter;void *end;
        struct NailArenaPool *next;
} NailArenaPool;
typedef struct NailArena_{
        NailArenaPool *current;                
        size_t blocksize;
} NailArena ;

void *n_malloc(NailArena *arena, size_t size)
{
        void *retval;
        if(arena->current->end - arena->current->iter <= size){
                size_t siz = arena->blocksize;
                if(size>siz)
                        siz = size + sizeof(NailArenaPool);
                NailArenaPool *newpool  = malloc(siz);
                if(!newpool) return NULL;
                newpool->end = (void *)((char *)newpool + siz);
                newpool->iter = (void*)(newpool+1);
                newpool->next = arena->current;
                arena->current= newpool;
        }
        retval = arena->current->iter;
        arena->current->iter += size;
        return retval;
}

int NailArena_init(NailArena *arena, size_t blocksize){
        if(blocksize< 2*sizeof(NailArena))
                blocksize = 2*sizeof(NailArena);
        arena->current = malloc(blocksize);
        if(!arena->current) return 0;
        arena->current->next = NULL;
        arena->current->iter = arena->current + 1;
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
}
//Returns the pointer where the taken choice is supposed to go.

#define parser_fail(i) __builtin_expect(i<0,0)



#include <stdint.h>
 #include <string.h>
#include <assert.h>
typedef struct number number;
typedef struct ipaddr ipaddr;
typedef uint8_t alnum;
typedef struct domain domain;
typedef struct definition definition;
typedef struct zone zone;
struct number{
uint8_t*elem;
 size_t count;
};
struct ipaddr{
number*elem;
 size_t count;
};
struct domain{
struct {
alnum*elem;
 size_t count;
}*elem;
 size_t count;
};
struct definition{
domain hostname;
struct {
 enum  {NX,CNAME,MX,A} N_type; 
union {
domain NX;
domain CNAME;
domain MX;
domain A;
};
} rr;
}
;
struct zone{
definition*elem;
 size_t count;
};

static pos packrat_WHITE(const uint8_t *data, pos off, pos max);
static pos packrat_number(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_ipaddr(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_alnum(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_domain(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_definition(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_zone(n_trace *trace, const uint8_t *data, pos off, pos max);
static pos packrat_WHITE(const uint8_t *data, pos off, pos max){
constmany_0_repeat:
{
pos backtrack = off;if(off + 8> max) {goto cunion_1_1;}
if( read_unsigned_bits(data,off,8)!= ' '){goto cunion_1_1;}off += 8;
goto cunion_1_succ;
cunion_1_1:
off = backtrack;
if(off + 8> max) {goto cunion_1_2;}
if( read_unsigned_bits(data,off,8)!= '\n'){goto cunion_1_2;}off += 8;
goto cunion_1_succ;
cunion_1_2:
off = backtrack;
goto constmany_1_end;
cunion_1_succ:;
}
goto constmany_0_repeat;
constmany_1_end:
return off;
fail: return -1;
}static pos packrat_number(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
{
pos many = n_tr_memo_many(trace);
pos count = 0;
succ_repeat_0:
if(off + 8> max) {goto fail_repeat_0;}
{
 uint64_t val = read_unsigned_bits(data,off,8);
if(val>'9'|| val < '0'){goto fail_repeat_0;}
}
off +=8;
count++;
 goto succ_repeat_0;
fail_repeat_0:
n_tr_write_many(trace,many,count);
}
return off;
fail:
 return -1;
}
static pos packrat_ipaddr(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
{
pos many = n_tr_memo_many(trace);
pos count = 0;
succ_repeat_1:
if(count>0){
if(off + 8> max) {goto fail_repeat_1;}
if( read_unsigned_bits(data,off,8)!= '.'){goto fail_repeat_1;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto fail_repeat_1;}
}
i = packrat_number(trace,data,off,max);if(parser_fail(i)){goto fail_repeat_1;}
else{off=i;}
count++;
 goto succ_repeat_1;
fail_repeat_1:
n_tr_write_many(trace,many,count);
if(count < 1){goto fail;}
}
return off;
fail:
 return -1;
}
static pos packrat_alnum(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
{pos backtrack = off;
pos choice_begin = n_tr_begin_choice(trace); 
pos choice;
if(parser_fail(choice_begin)) {goto fail;}
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_2_1_out;}
{
 uint64_t val = read_unsigned_bits(data,off,8);
if(val>'z'|| val < 'a'){goto choice_2_1_out;}
}
off +=8;
n_tr_pick_choice(trace,choice_begin,1,choice);goto choice_2_succ;
choice_2_1_out:
off = backtrack;
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_2_2_out;}
{
 uint64_t val = read_unsigned_bits(data,off,8);
if(val>'9'|| val < '0'){goto choice_2_2_out;}
}
off +=8;
n_tr_pick_choice(trace,choice_begin,2,choice);goto choice_2_succ;
choice_2_2_out:
off = backtrack;
goto fail;
choice_2_succ: ;
}return off;
fail:
 return -1;
}
static pos packrat_domain(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
{
pos many = n_tr_memo_many(trace);
pos count = 0;
succ_repeat_2:
if(count>0){
if(off + 8> max) {goto fail_repeat_2;}
if( read_unsigned_bits(data,off,8)!= '.'){goto fail_repeat_2;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto fail_repeat_2;}
}
{
pos many = n_tr_memo_many(trace);
pos count = 0;
succ_repeat_2:
i = packrat_alnum(trace,data,off,max);if(parser_fail(i)){goto fail_repeat_2;}
else{off=i;}
count++;
 goto succ_repeat_2;
fail_repeat_2:
n_tr_write_many(trace,many,count);
if(count < 1){goto fail_repeat_2;}
}
count++;
 goto succ_repeat_3;
fail_repeat_3:
n_tr_write_many(trace,many,count);
if(count < 1){goto fail;}
}
return off;
fail:
 return -1;
}
static pos packrat_definition(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
i = packrat_domain(trace,data,off,max);if(parser_fail(i)){goto fail;}
else{off=i;}
{pos backtrack = off;
pos choice_begin = n_tr_begin_choice(trace); 
pos choice;
if(parser_fail(choice_begin)) {goto fail;}
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_3_NX_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_NX_out;}off += 8;
if(off + 8> max) {goto choice_3_NX_out;}
if( read_unsigned_bits(data,off,8)!= 'N'){goto choice_3_NX_out;}off += 8;
if(off + 8> max) {goto choice_3_NX_out;}
if( read_unsigned_bits(data,off,8)!= 'X'){goto choice_3_NX_out;}off += 8;
if(off + 8> max) {goto choice_3_NX_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_NX_out;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto choice_3_NX_out;}
i = packrat_domain(trace,data,off,max);if(parser_fail(i)){goto choice_3_NX_out;}
else{off=i;}
n_tr_pick_choice(trace,choice_begin,NX,choice);goto choice_3_succ;
choice_3_NX_out:
off = backtrack;
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= 'C'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= 'N'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= 'A'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= 'M'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= 'E'){goto choice_3_CNAME_out;}off += 8;
if(off + 8> max) {goto choice_3_CNAME_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_CNAME_out;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto choice_3_CNAME_out;}
i = packrat_domain(trace,data,off,max);if(parser_fail(i)){goto choice_3_CNAME_out;}
else{off=i;}
n_tr_pick_choice(trace,choice_begin,CNAME,choice);goto choice_3_succ;
choice_3_CNAME_out:
off = backtrack;
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_3_MX_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_MX_out;}off += 8;
if(off + 8> max) {goto choice_3_MX_out;}
if( read_unsigned_bits(data,off,8)!= 'M'){goto choice_3_MX_out;}off += 8;
if(off + 8> max) {goto choice_3_MX_out;}
if( read_unsigned_bits(data,off,8)!= 'X'){goto choice_3_MX_out;}off += 8;
if(off + 8> max) {goto choice_3_MX_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_MX_out;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto choice_3_MX_out;}
i = packrat_domain(trace,data,off,max);if(parser_fail(i)){goto choice_3_MX_out;}
else{off=i;}
n_tr_pick_choice(trace,choice_begin,MX,choice);goto choice_3_succ;
choice_3_MX_out:
off = backtrack;
choice = n_tr_memo_choice(trace);
if(off + 8> max) {goto choice_3_A_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_A_out;}off += 8;
if(off + 8> max) {goto choice_3_A_out;}
if( read_unsigned_bits(data,off,8)!= 'A'){goto choice_3_A_out;}off += 8;
if(off + 8> max) {goto choice_3_A_out;}
if( read_unsigned_bits(data,off,8)!= ':'){goto choice_3_A_out;}off += 8;
if(parser_fail(n_tr_const(trace,off))){goto choice_3_A_out;}
i = packrat_domain(trace,data,off,max);if(parser_fail(i)){goto choice_3_A_out;}
else{off=i;}
n_tr_pick_choice(trace,choice_begin,A,choice);goto choice_3_succ;
choice_3_A_out:
off = backtrack;
goto fail;
choice_3_succ: ;
}return off;
fail:
 return -1;
}
static pos packrat_zone(n_trace *trace, const uint8_t *data, pos off, pos max){
pos i;
{
pos many = n_tr_memo_many(trace);
pos count = 0;
succ_repeat_4:
i = packrat_definition(trace,data,off,max);if(parser_fail(i)){goto fail_repeat_4;}
else{off=i;}
count++;
 goto succ_repeat_4;
fail_repeat_4:
n_tr_write_many(trace,many,count);
if(count < 1){goto fail;}
}
return off;
fail:
 return -1;
}

static pos bind_number(NailArena *arena,number*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);static pos bind_ipaddr(NailArena *arena,ipaddr*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);static pos bind_alnum(NailArena *arena,alnum*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);static pos bind_domain(NailArena *arena,domain*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);static pos bind_definition(NailArena *arena,definition*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);static pos bind_zone(NailArena *arena,zone*out,const uint8_t *data, pos off, pos **trace,  pos * trace_begin);
static pos bind_number(NailArena *arena,number*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
{ /*ARRAY*/ 
 pos save = 0;out->count=*(tr++);
save = *(tr++);
out->elem= n_malloc(arena,out->count* sizeof(*out->elem));
if(!out->elem){return 0;}
for(pos i1=0;i1<out->count;i1++){out->elem[i1]=read_unsigned_bits(data,off,8);
off += 8;
}
tr = trace_begin + save;
}*trace = tr;return off;}number*parse_number(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
number* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_number(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_number(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
static pos bind_ipaddr(NailArena *arena,ipaddr*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
{ /*ARRAY*/ 
 pos save = 0;out->count=*(tr++);
save = *(tr++);
out->elem= n_malloc(arena,out->count* sizeof(*out->elem));
if(!out->elem){return 0;}
for(pos i2=0;i2<out->count;i2++){if(i>0){fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
}off = bind_number(arena,&out->elem[i2], data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}}
tr = trace_begin + save;
}*trace = tr;return off;}ipaddr*parse_ipaddr(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
ipaddr* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_ipaddr(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_ipaddr(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
static pos bind_alnum(NailArena *arena,alnum*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);
switch(*(tr++)){
case 1:
tr = trace_begin + *tr;
out[0]=read_unsigned_bits(data,off,8);
off += 8;
break;
case 2:
tr = trace_begin + *tr;
out[0]=read_unsigned_bits(data,off,8);
off += 8;
break;
default: assert(!"Error"); exit(-1);}*trace = tr;return off;}alnum*parse_alnum(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
alnum* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_alnum(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_alnum(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
static pos bind_domain(NailArena *arena,domain*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
{ /*ARRAY*/ 
 pos save = 0;out->count=*(tr++);
save = *(tr++);
out->elem= n_malloc(arena,out->count* sizeof(*out->elem));
if(!out->elem){return 0;}
for(pos i3=0;i3<out->count;i3++){if(i>0){fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
}fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
{ /*ARRAY*/ 
 pos save = 0;out->elem[i3].count=*(tr++);
save = *(tr++);
out->elem[i3].elem= n_malloc(arena,out->elem[i3].count* sizeof(*out->elem[i3].elem));
if(!out->elem[i3].elem){return 0;}
for(pos i4=0;i4<out->elem[i3].count;i4++){off = bind_alnum(arena,&out->elem[i3].elem[i4], data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}}
tr = trace_begin + save;
}}
tr = trace_begin + save;
}*trace = tr;return off;}domain*parse_domain(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
domain* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_domain(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_domain(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
static pos bind_definition(NailArena *arena,definition*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;off = bind_domain(arena,&out->hostname, data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}fprintf(stderr,"%d = choice %d %d\n",tr-trace_begin, tr[0], tr[1]);switch(*(tr++)){
case NX:
tr = trace_begin + *tr;
out->rr.N_type= NX;
fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
off = bind_domain(arena,&out->rr.NX, data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}break;
case CNAME:
tr = trace_begin + *tr;
out->rr.N_type= CNAME;
fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
off = bind_domain(arena,&out->rr.CNAME, data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}break;
case MX:
tr = trace_begin + *tr;
out->rr.N_type= MX;
fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
off = bind_domain(arena,&out->rr.MX, data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}break;
case A:
tr = trace_begin + *tr;
out->rr.N_type= A;
fprintf(stderr,"%d = const %d\n",tr-trace_begin, *tr);
off  = *tr;
tr++;
off = bind_domain(arena,&out->rr.A, data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}break;
default: assert("BUG");}*trace = tr;return off;}definition*parse_definition(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
definition* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_definition(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_definition(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
static pos bind_zone(NailArena *arena,zone*out,const uint8_t *data, pos off, pos **trace ,  pos * trace_begin){
 pos *tr = *trace;fprintf(stderr,"%d = many %d %d\n",tr-trace_begin, tr[0], tr[1]);
{ /*ARRAY*/ 
 pos save = 0;out->count=*(tr++);
save = *(tr++);
out->elem= n_malloc(arena,out->count* sizeof(*out->elem));
if(!out->elem){return 0;}
for(pos i5=0;i5<out->count;i5++){off = bind_definition(arena,&out->elem[i5], data,off,&tr,trace_begin);if(parser_fail(off)){return -1;}}
tr = trace_begin + save;
}*trace = tr;return off;}zone*parse_zone(NailArena *arena, const uint8_t *data, size_t size){
n_trace trace;
pos *tr_ptr;
 pos pos;
zone* retval;
n_trace_init(&trace,4096,4096);
tr_ptr = trace.trace;if(size*8 != packrat_zone(&trace,data,0,size*8)) return NULL;retval = n_malloc(arena,sizeof(*retval));
if(!retval) return NULL;
if(bind_zone(arena,retval,data,0,&tr_ptr,trace.trace) < 0) return NULL;
n_trace_release(&trace);
return retval;}
#include <hammer/hammer.h>
#include <hammer/internal.h>
void gen_WHITE(HBitWriter* out);void gen_number(HBitWriter *out,number * val);void gen_ipaddr(HBitWriter *out,ipaddr * val);void gen_alnum(HBitWriter *out,alnum * val);void gen_domain(HBitWriter *out,domain * val);void gen_definition(HBitWriter *out,definition * val);void gen_zone(HBitWriter *out,zone * val);void gen_WHITE(HBitWriter* out){
h_bit_writer_put(out,' ',8);
}void gen_number(HBitWriter *out,number * val){for(int i0=0;i0<val->count;i0++){h_bit_writer_put(out,val->elem[i0],8);}}void gen_ipaddr(HBitWriter *out,ipaddr * val){for(int i1=0;i1<val->count;i1++){if(i1!= 0){h_bit_writer_put(out,'.',8);
}gen_number(out,&val->elem[i1]);}}void gen_alnum(HBitWriter *out,alnum * val){h_bit_writer_put(out,val[0],8);}void gen_domain(HBitWriter *out,domain * val){for(int i2=0;i2<val->count;i2++){if(i2!= 0){h_bit_writer_put(out,'.',8);
}for(int i3=0;i3<val->elem[i2].count;i3++){gen_alnum(out,&val->elem[i2].elem[i3]);}}}void gen_definition(HBitWriter *out,definition * val){gen_domain(out,&val->hostname);switch(val->rr.N_type){
case NX:
h_bit_writer_put(out,':',8);
h_bit_writer_put(out,'N',8);
h_bit_writer_put(out,'X',8);
h_bit_writer_put(out,':',8);
gen_domain(out,&val->rr.NX);break;
case CNAME:
h_bit_writer_put(out,':',8);
h_bit_writer_put(out,'C',8);
h_bit_writer_put(out,'N',8);
h_bit_writer_put(out,'A',8);
h_bit_writer_put(out,'M',8);
h_bit_writer_put(out,'E',8);
h_bit_writer_put(out,':',8);
gen_domain(out,&val->rr.CNAME);break;
case MX:
h_bit_writer_put(out,':',8);
h_bit_writer_put(out,'M',8);
h_bit_writer_put(out,'X',8);
h_bit_writer_put(out,':',8);
gen_domain(out,&val->rr.MX);break;
case A:
h_bit_writer_put(out,':',8);
h_bit_writer_put(out,'A',8);
h_bit_writer_put(out,':',8);
gen_domain(out,&val->rr.A);break;
}{/*Context-rewind*/
 HBitWriter end_of_struct= *out;
out->index = end_of_struct.index;
out->bit_offset = end_of_struct.bit_offset;
}}void gen_zone(HBitWriter *out,zone * val){for(int i4=0;i4<val->count;i4++){gen_definition(out,&val->elem[i4]);}}
