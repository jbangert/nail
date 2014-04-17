
struct NailArenaPool;
typedef struct NailArena_ {
    struct NailArenaPool *current;
    size_t blocksize;
} NailArena ;
int NailArena_init(NailArena *arena,size_t blocksize);
int NailArena_release(NailArena *arena);
struct NailStream {
    const uint8_t *data;
    size_t size;
    size_t pos;
    signed char bit_offset;
};
typedef struct NailStream NailStream;
typedef size_t NailStreamPos;
zip*parse_zip(NailArena *arena, const uint8_t *data, size_t size);
foo*parse_foo(NailArena *arena, const uint8_t *data, size_t size);
