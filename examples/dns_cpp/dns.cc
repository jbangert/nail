#include "dns.nail.hh"
#include "nail/memstream.hh"
#include <new>
template <typename str> struct dnscompress_parse{
  typedef NailOutBufferStream out_1_t;
  static int f(NailArena *tmp, out_1_t **str_decompressed, str *current){
    //This is very inefficient, as far as coroutines go, but oh well
    const int64_t labelsize= 256; //rfc 1035, section 2.3.4
    uint8_t *buf = (uint8_t *)n_malloc(tmp,labelsize);
    uint8_t *out = buf;
    if(!buf)
      return -1;
    typename str::pos_t end_of_labels;
    int first_label = 1;
    size_t size = 0;
    while(1){
      uint8_t highbyte;
      if(n_fail(current->check(8)))
        return -1;
      highbyte = current->read_unsigned_big(8);
      if(highbyte >= 192){
        if(first_label){
          first_label =0;
          end_of_labels = current->getpos();
        }
        if(n_fail(current->check(8))) return -1;
        int offset = ((highbyte & 63) << 8) | current->read_unsigned_big(8);
        if(n_fail(current->repositionOffset(offset,0))) return -1;
      }
      if(highbyte == 0){
        if(++size >= labelsize) return -1;
        *out++ = highbyte;
        break;
      }
      if(highbyte >= 64)
        return -1;
      size+= highbyte+1;
      if(size>= labelsize)
        return -1;
      *out++ = highbyte;
      for(;highbyte; highbyte--){
        if(n_fail(current->check(8))) return -1;
        *out= current->read_unsigned_big(8);
        out++;
      }
    }
    (*str_decompressed) = (out_1_t*)n_malloc(tmp, sizeof(**str_decompressed));
    if(!*str_decompressed){
      return -1;
    }
    new(*str_decompressed) out_1_t();

    (*str_decompressed)->SetBuffer(buf,out-buf);
    return 0;
  }
};

int dnscompress_generate(NailArena *tmp,NailOutStream *str_compressed, NailOutStream *current){
        if(NailOutStream_grow(current, 8*str_compressed->pos)) return -1;
        memcpy((char *)current->data + current->pos, str_compressed->data, str_compressed->pos);
        current->pos += str_compressed->pos;
        return 0;
}


#include "dns.nail.cc"
