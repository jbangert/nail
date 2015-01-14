#include <stdint.h>
#include <string.h>
#include <assert.h>
enum N_types {_NAIL_NULL,A,MX,CNAME,NS};
typedef struct labels labels;
typedef struct compressed compressed;
typedef struct question question;
typedef struct answer answer;
typedef struct dnspacket dnspacket;
typedef struct number number;
typedef struct ipaddr ipaddr;
typedef uint8_t alnum;
typedef struct domain domain;
typedef struct definition definition;
typedef struct zone zone;
struct labels {
    struct {
        struct {
            uint8_t*elem;
            size_t count;
        } label;
    }
    *elem;
    size_t count;
};
struct compressed {
    labels labels;
}
;
struct question {
    compressed labels;
    uint16_t qtype;
    uint16_t qclass;
}
;
struct answer {
    compressed labels;
    uint16_t rtype;
    uint16_t class;
    uint32_t ttl;
    struct {
        uint8_t*elem;
        size_t count;
    } rdata;
}
;
struct dnspacket {
    uint16_t id;
    uint8_t qr;
    uint8_t opcode;
    uint8_t aa;
    uint8_t tc;
    uint8_t rd;
    uint8_t ra;
    uint8_t rcode;
    struct {
        question*elem;
        size_t count;
    } questions;
    struct {
        answer*elem;
        size_t count;
    } responses;
    struct {
        answer*elem;
        size_t count;
    } authority;
    struct {
        answer*elem;
        size_t count;
    } additional;
}
;
struct number {
    uint8_t*elem;
    size_t count;
};
struct ipaddr {
    number*elem;
    size_t count;
};
struct domain {
    struct {
        alnum*elem;
        size_t count;
    }*elem;
    size_t count;
};
struct definition {
    domain hostname;
    struct {
        enum N_types N_type;
        union {
            domain ns;
            domain cname;
            domain mx;
            domain a;
        };
    } rr;
}
;
struct zone {
    definition*elem;
    size_t count;
};



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
labels*parse_labels(NailArena *arena, NailStream *data);
compressed*parse_compressed(NailArena *arena, NailStream *data);
question*parse_question(NailArena *arena, NailStream *data);
answer*parse_answer(NailArena *arena, NailStream *data);
dnspacket*parse_dnspacket(NailArena *arena, NailStream *data);
number*parse_number(NailArena *arena, NailStream *data);
ipaddr*parse_ipaddr(NailArena *arena, NailStream *data);
alnum*parse_alnum(NailArena *arena, NailStream *data);
domain*parse_domain(NailArena *arena, NailStream *data);
definition*parse_definition(NailArena *arena, NailStream *data);
zone*parse_zone(NailArena *arena, NailStream *data);
extern int dnscompress_parse(NailArena *tmp,NailStream *str_decompressed,NailStream *current);

int gen_labels(NailArena *tmp_arena,NailStream *out,labels * val);
int gen_compressed(NailArena *tmp_arena,NailStream *out,compressed * val);
extern  int dnscompress_generate(NailArena *tmp_arena,NailStream *str_decompressed,NailStream *str_current);
int gen_question(NailArena *tmp_arena,NailStream *out,question * val);
int gen_answer(NailArena *tmp_arena,NailStream *out,answer * val);
int gen_dnspacket(NailArena *tmp_arena,NailStream *out,dnspacket * val);
int gen_WHITE(NailStream* str_current);
int gen_number(NailArena *tmp_arena,NailStream *out,number * val);
int gen_ipaddr(NailArena *tmp_arena,NailStream *out,ipaddr * val);
int gen_alnum(NailArena *tmp_arena,NailStream *out,alnum * val);
int gen_domain(NailArena *tmp_arena,NailStream *out,domain * val);
int gen_definition(NailArena *tmp_arena,NailStream *out,definition * val);
int gen_zone(NailArena *tmp_arena,NailStream *out,zone * val);


