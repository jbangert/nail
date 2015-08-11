#include <nail/memstream.hh>
#include <new>
template <typename str> struct offset_parse{};
template <> struct offset_parse<NailMemStream> {
  typedef NailMemStream out_1_t;
  template<typename siz_t> static int f(NailArena *tmp, NailMemStream **out_frag, NailMemStream *in_total, siz_t *offset){
    if(*offset > in_total->getSize())
      return -1;
    *out_frag = (NailMemStream *)n_malloc(tmp, sizeof(NailMemStream));
    if(!*out_frag)
      return -1;
    new((void *)*out_frag) NailMemStream(in_total->getBuf() + *offset,in_total->getSize() - *offset);
    return 0;
  }
};
template <typename siz_t> int offset_generate(NailArena *tmp, NailOutStream *fragment, siz_t *offset, NailOutStream *current){
        if(NailOutStream_grow(current,fragment->pos)<0)
                return -1;
        if(current->bit_offset)
                return -1;
        memcpy(current->data + current->pos, fragment->data, fragment->pos);
        *offset = current->pos;
        current->pos += fragment->pos;
        return 0;
}
