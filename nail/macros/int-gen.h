#include <stdlib.h>
#include <string.h>
typedef struct n_outbuf{
        char *begin; 
        size_t write,size;
        size_t bit_offset;
} n_outbuf;
static void writebits(n_outbuf *out, size_t size, void *data)
{  
        while(out->write + size >= out->size){              \
                char *tmp = malloc(2*out->size);                \
                if(!tmp) exit(0);                               \
                memcpy(tmp,out->begin,size);                   \
                free(out->begin);                               \
                out->begin=tmp;                                 \
                out->size = 2*out->size;                        \
        }                                                       \
        memcpy(&out->begin,data,size);                          \
        out->write += size;                                     \        
}




#define h_bits(x,sign) write_h_bits(x,sign,
#define h_uint8()  writebits(out, sizeof(uint8_t), &
#define h_uint16() writebits(out, sizeof(uint16_t),&
#define h_uint32() writebits(out, sizeof(uint32_t),&
#define h_uint64() writebits(out, sizeof(uint64_t),&
#define h_sint8()  writebits(out, sizeof(int8_t),&
#define h_sint16() writebits(out, sizeof(int16_t),&
#define h_sint32() writebits(out, sizeof(int32_t),&
#define h_sint64() writebits(out, sizeof(int64_t),&

#define N_STRUCT(inner)                                           \
        {                                                           \
                __typeof__(val) str =  val;                           \
                inner                                                 \
        }
#define N_FIELD(name,inner) {const typeof(str->name) *val = &str->name; inner;}

#define  N_ARRAY(combinator,inner)                                      \
        {                                                               \
        int i;                                                        \
        __typeof__(val->elem) elem =val ;                                   \
        for(i=0;i<val->count;i++){                                    \
                __typeof__(elem) val = elem + i;                      \
                inner;                                                \
        }                                                             \
        }
        


//#define NX_LENGTHVALUE_HACK(lengthp,elemp) N_ARRAY(elemp,h_length_value)


#define N_CONSTANT(inner) inner;
#define N_SCALAR(cast,type,parser) parser val)
#define N_OPTIONAL(inner) if(val){inner;}
#define N_PARSER(name) write_buf_ ## name(out,val);

#define N_DEFPARSER(name,inner)                                         \
        static void write_buf_##name (n_outbuf *out,const name *val) {  \
                inner;                                                  \
                }                                                       \
        char * name ##write(const name *val, size_t * len)     \
                {                                               \
                n_outbuf buf;                                   \
                buf.begin = malloc(8192);                       \
                buf.write = 0;                                  \
                buf.size = 8192;                                \
                write_buf_ ##name (&buf,val);                   \
                *len = buf.write;                       \
                return buf.begin;                       \
        }
        
