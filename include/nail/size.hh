#include <nail/memstream.hh>
#include <new>
template <typename str> struct size_parse{
};
template <> struct size_parse<NailMemStream>{
  typedef NailMemStream  out_1_t;
  template<typename siz_t> static int f(NailArena *tmp, NailMemStream *out_frag, NailMemStream *in_total, siz_t *size){
    NailMemStream::pos_t p = in_total->getpos();
    if(p.bit_offset != 0)
      return -1;
    if(in_total->getSize() < *size )
      return -1;
    size_t s = in_total->getSize() - *size;
    if(*size < s)
      s = *size;
    out_frag = (NailMemStream *)n_malloc(tmp,sizeof(NailMemStream));
    if(!out_frag)
      return -1;
    new((void *)out_frag) NailMemStream(in_total->getBuf(),s);
    return 0;
  }
};
template<typename siz_t>
 int size_generate(NailArena *tmp, NailOutStream *fragment, NailOutStream *current, siz_t *size){ 
        //TODO: Refactor into a stream append library!
        if(NailOutStream_grow(current, 8*fragment->pos)<0)
                return -1;
        if(current->bit_offset)
                return -1;
        memcpy(current->data + current->pos, fragment->data, fragment->pos);
        current->pos += fragment->pos;
        *size = fragment->pos;
        return 0;
}
