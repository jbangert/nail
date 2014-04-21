
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
int NailOutStream_new(NailStream *str,size_t siz);
void NailOutStream_release(NailStream *str);
const uint8_t * NailOutStream_buffer(NailStream *str,size_t *siz);
