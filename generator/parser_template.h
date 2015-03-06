
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

struct NailOutStream {
  uint8_t *data;
  size_t size;
  size_t pos;
  signed char bit_offset;
};
typedef struct NailStream NailStream;
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

