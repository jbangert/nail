#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#define parser_fail(i) __builtin_expect((i)<0,0)

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
static NailOutStreamPos   NailOutStream_getpos(NailOutStream *stream){
  return (stream->pos << 3) + stream->bit_offset; //TODO: Overflow potential!
}


int NailOutStream_init(NailOutStream *out,size_t siz){
        out->data = ( uint8_t *)malloc(siz);
        if(!out->data)
                return -1;
        out->pos = 0;
        out->bit_offset = 0;
        out->size = siz;
        return 0;
}
void NailOutStream_release(NailOutStream *out){
  free((void *)out->data);
        out->data = NULL;
}
const uint8_t * NailOutStream_buffer(NailOutStream *str,size_t *siz){
        if(str->bit_offset)
                return NULL;
        *siz =  str->pos;
        return str->data;
}
//TODO: Perhaps use a separate structure for output streams?
int NailOutStream_grow(NailOutStream *stream, size_t count){
  if(stream->pos + (count>>3) + 1 >= stream->size){
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
static int NailOutStream_write(NailOutStream *stream,uint64_t data, size_t count){
        if(parser_fail(NailOutStream_grow(stream, count))){
                return -1;
        }
        uint8_t *streamdata = (uint8_t *)stream->data;
        while(count>0){
                if(stream->bit_offset == 0 && (count & 7) == 0){
                        count -= 8;
                        streamdata[stream->pos] = (data >> count );
                        stream->pos++;
                }
                else{
                        count--;
                        if((data>>count)&1)
                                streamdata[stream->pos] |= 1 << (7-stream->bit_offset);
                        else 
                                streamdata[stream->pos] &= ~(1<< (7-stream->bit_offset));
                        stream->bit_offset++;
                        if(stream->bit_offset>7){
                                stream->pos++;
                                stream->bit_offset= 0;
                        }
                }
        }
        return 0;
}

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
                if(!newpool){
                  longjmp(*arena->error_ret, -1);
                }
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

int NailArena_init(NailArena *arena, size_t blocksize, jmp_buf *err){
        if(blocksize< 2*sizeof(NailArena))
                blocksize = 2*sizeof(NailArena);
        arena->current = (NailArenaPool*)malloc(blocksize);
        if(!arena->current) return 0;
        arena->current->next = NULL;
        arena->current->iter = (char *)(arena->current + 1);
        arena->current->end = (char *) arena->current + blocksize;
        arena->blocksize = blocksize;
        arena->error_ret = err;
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








