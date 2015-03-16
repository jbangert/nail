#include <stdint.h>
#include <string.h>
#include <assert.h>
enum N_types {_NAIL_NULL,FRAGMENT,ROUTING,UDP,ICMP,HOPBYHOP};
typedef struct ip6contents ip6contents;
typedef struct ip6addr ip6addr;
typedef struct ip6packet ip6packet;
typedef struct ethernet ethernet;
typedef struct foo foo;
struct ip6contents {
    enum N_types N_type;
    union {
        struct {
        }
        hopbyhop;
        struct {
        }
        icmp;
        struct {
            uint16_t source;
            uint16_t destination;
            uint16_t checksum;
            struct {
                uint8_t*elem;
                size_t count;
            } payload;
        }
        udp;
        struct {
        }
        routing;
        struct {
        }
        fragment;
    };
};
struct ip6addr {
    uint64_t net;
    uint64_t host;
}
;
struct ip6packet {
    uint8_t diffserv;
    uint8_t ecn;
    uint32_t flow;
    uint8_t hops;
    ip6addr saddr;
    ip6addr daddr;
    ip6contents payload;
}
;
struct ethernet {
    uint64_t dest;
    uint64_t src;
    uint16_t ethertype;
    ip6packet ip;
}
;
struct foo {
    uint16_t major;
    uint16_t minor;
    uint32_t zone;
    uint32_t figs;
    uint32_t snaplen;
    uint32_t network;
    struct {
        struct {
            uint32_t sec;
            uint32_t usec;
            ethernet packet;
        }
        *elem;
        size_t count;
    } packets;
}
;



struct NailArenaPool;
typedef struct NailArena_ {
    struct NailArenaPool *current;
    size_t blocksize;
} NailArena ;
typedef struct {
    struct NailArenaPool *pool;
    char *iter;
} NailArenaPos;
NailArenaPos n_arena_save(NailArena *arena);
void n_arena_restore(NailArena *arena, NailArenaPos p);

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

ip6contents*parse_ip6contents(NailArena *arena, NailStream *data);
ip6addr*parse_ip6addr(NailArena *arena, NailStream *data);
ip6packet*parse_ip6packet(NailArena *arena, NailStream *data);
ethernet*parse_ethernet(NailArena *arena, NailStream *data);
foo*parse_foo(NailArena *arena, NailStream *data);

int gen_ip6contents(NailArena *tmp_arena,NailOutStream *out,ip6contents * val,uint8_t* dep_type);
int gen_ip6addr(NailArena *tmp_arena,NailOutStream *out,ip6addr * val);
int gen_ip6packet(NailArena *tmp_arena,NailOutStream *out,ip6packet * val);
int gen_ethernet(NailArena *tmp_arena,NailOutStream *out,ethernet * val);
int gen_foo(NailArena *tmp_arena,NailOutStream *out,foo * val);


