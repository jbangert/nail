#include "dns.nail.hh"
#include "nail/memstream.hh"

template <typename str> struct dnscompress_parse{
  typedef NailOutBufferStream decompressed;
  static int f(NailArena *tmp, decompressed *str_decompressed, str *current){
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
    str_decompressed->SetBuffer(buf,out-buf);
    return 0;
  }
};

#include "dns.nail.cc"
/*
int dnscompress_parse(NailArena *tmp,NailStream *str_decompressed, NailStream *current){
        if(current->bit_offset) return -1;
        size_t begin_labels = current->pos;
        size_t pos = begin_labels;
        int first_label = 1;
        //First, figure out how large the stream should be.
        size_t size =0;
        while(1){
                uint8_t highbyte;
                if(pos >= current->size) return -1;
                size++;
                highbyte = current->data[pos++];
                if(highbyte == 0){
                        if(first_label){
                                first_label = 0;
                                current->pos = pos;
                        }
                        break;
                }
                if(highbyte >= 192){
                        if(pos >= current->size) return -1;
                        if(first_label){
                                first_label = 0;
                                current->pos = pos+1;
                        }
                        pos = highbyte & 63 << 8 | current->data[pos];
                } else {
                        size += highbyte;
                        pos += highbyte;
                }                
        }
        str_decompressed->data = n_malloc(tmp,size);
        if(!str_decompressed->data) return -1;
        str_decompressed->pos = 0;
        str_decompressed->bit_offset = 0;
        str_decompressed->size = size;
        pos = begin_labels;
        while(1){
                uint8_t highbyte;
                if(pos >= current->size) return -1;
                highbyte = current->data[pos];
                if(highbyte >= 192){                     
                        if(pos >= current->size) return -1;
                        pos = highbyte & 63 << 8 | current->data[pos+1];
                        continue;
                }
                memcpy(str_decompressed->data + str_decompressed->pos, current->data + pos, highbyte + 1);
                str_decompressed->pos += highbyte+1;
                pos += highbyte +1;
                if(highbyte == 0){
                        break;
                }
        }
        str_decompressed->pos = 0;
        return 0;
        }*/

int dnscompress_generate(NailArena *tmp,NailStream *str_compressed, NailStream *current){
        if(NailOutStream_grow(current, 8*str_compressed->pos)) return -1;
        memcpy((char *)current->data + current->pos, str_compressed->data, str_compressed->pos);
        current->pos += str_compressed->pos;
        return 0;
}

