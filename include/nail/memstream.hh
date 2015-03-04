
class NailMemStream{
  friend class NailOutBufferStream;
public:
  typedef struct {
    size_t pos;
    signed char bit_offset;
  } pos_t;
protected:
  uint8_t *data;
  pos_t pos;
  size_t size;
public:
  const uint8_t *getBuf(){ return data;}
  const size_t getSize(){ return size;}
  NailMemStream(uint8_t *data, size_t size){
    this->data = data;
    this->size = size;
    pos.pos = 0;
    pos.bit_offset = 0;
  }
  int check(unsigned count){
      if(size - (count>>3) - ((pos.bit_offset + count & 7)>>3) < pos.pos)
        return -1;
      return 0;
  }
  pos_t getpos() { return pos; }
  void rewind(pos_t &p) { pos = p;}
  int repositionOffset(size_t bytes, size_t bits){
    pos.pos = bytes;
    pos.bit_offset = bits;
    return check(0);
  }
  uint64_t read_unsigned_big(unsigned count){
        uint64_t retval = 0;
        unsigned int out_idx=count;
        size_t pos = this->pos.pos;
        char bit_offset = this->pos.bit_offset;
        const uint8_t *data = this->data;
        //TODO: Implement little endian too
        //Count LSB to MSB
        while(count>0) {
                if(bit_offset == 0 && (count &7) ==0) {
                        out_idx-=8;
                        retval|= data[pos] << out_idx;
                        pos ++;
                        count-=8;
                }
                else{
                        //This can use a lot of performance love
//TODO: implement other endianesses
                        out_idx--;
                        retval |= ((data[pos] >> (7-bit_offset)) & 1) << out_idx;
                        count--;
                        bit_offset++;
                        if(bit_offset > 7){
                                bit_offset -= 8;
                                pos++;
                        }
                }
        }
        this->pos.pos = pos;
        this->pos.bit_offset = bit_offset;
    return retval;
  }
};
class NailOutBufferStream : public NailMemStream{
public:
  NailOutBufferStream(): NailMemStream(NULL,0){}
  void SetBuffer(uint8_t *data, size_t size){
    this->data = data;
    this->size = size;
  }
  void SetBuffer(NailMemStream *str){
    this->data = str->data;
    this->size = str->size;
    this->pos = str->pos;
  }
};
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
