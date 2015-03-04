#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define parser_fail(i) __builtin_expect(i,0)
typedef struct NailArenaPool{
        char *iter;char *end;
        struct NailArenaPool *next;
} NailArenaPool;
// free on backtrack?
typedef struct {
        struct NailArenaPool *pool;
        char *iter;
} NailArenaPos;
static NailArenaPos n_arena_save(NailArena *arena){
        NailArenaPos retval = {.pool = arena->current, .iter = arena->current->iter};
        return retval;
}
static void n_arena_restore(NailArena *arena, NailArenaPos p){
        arena->current = p.pool;
        arena->current->iter = p.iter;
        //memory will remain linked
}
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
