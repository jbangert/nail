
struct NailArenaPool;
typedef struct NailArena_ {
    struct NailArenaPool *current;
    size_t blocksize;
} NailArena ;
extern int NailArena_init(NailArena *arena,size_t blocksize);
extern int NailArena_release(NailArena *arena);
extern void *n_malloc(NailArena *arena, size_t size);
struct NailStream {
    const uint8_t *data;
    size_t size;
    size_t pos;
    signed char bit_offset;
};

typedef struct NailStream NailStream;
typedef size_t NailStreamPos;
static NailStream * NailStream_alloc(NailArena *arena) {
        return (NailStream *)n_malloc(arena, sizeof(NailStream));
} 
extern int NailOutStream_init(NailStream *str,size_t siz);
extern void NailOutStream_release(NailStream *str);
const uint8_t * NailOutStream_buffer(NailStream *str,size_t *siz);
extern int NailOutStream_grow(NailStream *stream, size_t count);

#define n_fail(i) __builtin_expect(i,0)
