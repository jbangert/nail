#include <setjmp.h>
struct NailArenaPool;
struct NailArena;
struct NailArenaPos;
typedef struct NailArena NailArena;
typedef struct NailArenaPos NailArenaPos;
NailArenaPos n_arena_save(NailArena *arena);
void n_arena_restore(NailArena *arena, NailArenaPos p);

struct NailArena {
  struct NailArenaPool *current;
  size_t blocksize;
  jmp_buf *error_ret; // XXX: Leaks memory on OOM. Keep a linked list of erroring arenas? 
};
struct NailArenaPos{
        struct NailArenaPool *pool;
        char *iter;
} ;
extern int NailArena_init(NailArena *arena,size_t blocksize, jmp_buf *error_return);
extern int NailArena_release(NailArena *arena);
extern void *n_malloc(NailArena *arena, size_t size);
struct NailStream {
    const uint8_t *data;
    size_t size;
    size_t pos;
    signed char bit_offset;
};

struct NailOutStream {
  uint8_t *data;
  size_t size;
  size_t pos;
  signed char bit_offset;
};
typedef struct NailStream NailStream;
typedef struct NailOutStream NailOutStream;
typedef size_t NailStreamPos;
typedef size_t NailOutStreamPos;
static NailStream * NailStream_alloc(NailArena *arena) {
        return (NailStream *)n_malloc(arena, sizeof(NailStream));
} 
static NailOutStream * NailOutStream_alloc(NailArena *arena) {
        return (NailOutStream *)n_malloc(arena, sizeof(NailOutStream));
} 
extern int NailOutStream_init(NailOutStream *str,size_t siz);
extern void NailOutStream_release(NailOutStream *str);
const uint8_t * NailOutStream_buffer(NailOutStream *str,size_t *siz);
extern int NailOutStream_grow(NailOutStream *stream, size_t count);

#define n_fail(i) __builtin_expect(i,0)
