#include "parse.hh"
#include <nail/size.hh>
/* template<typename siz_t> struct NailCoroFragmentStream {
private:
  typedef std::map<siz_t, NailMemStream> inner_t;
  std::map<siz_t, NailMemStream> streams;
public:
  typedef class {
    inner_t::iterator iter;
    NailMemStream::pos_t pos;
  } 
  };*/
template <typename str> struct ip_header_parse {
};
template <> struct ip_header_parse<NailMemStream> {
  typedef NailMemStream out_1_t;
  typedef NailMemStream out_2_t;
  typedef NailMemStream out_3_t;
  static int f(NailArena *tmp, NailMemStream **out_options,NailMemStream *in_current, uint8_t *ihl, uint16_t *ptr_checksum){
    if(n_fail(in_current->repositionOffset(*ihl * 4, 0))) return -1;
    *out_options = (NailMemStream *)n_malloc(tmp,sizeof(NailMemStream));
    new((void *)*out_options) NailMemStream(in_current->getBuf() + 20, *ihl * 4 - 20);
    
    uint32_t checksum = 0;
    uint16_t *field = (uint16_t *)in_current->getBuf();
    for(size_t l = *ihl * 4; l>0;l--){
      checksum += *field;
      field++;
    }
    while(checksum >> 16){
      checksum = (checksum & 0xFFFF)+ (checksum>>16);
    }
    if(checksum != 0) return -1;
    return 0;
  }
};
int ip_header_generate(NailArena *tmp,  const NailOutStream* in_options, NailOutStream *current, uint8_t *ihl, uint16_t *out_checksum)
{
  if(in_options->pos % 4 != 0) return -1; // XXX:: HEADER PADDING!
  if(in_options->pos + current->pos >= 64) return -1;
  if(n_fail(size_generate(tmp, in_options,  current, ihl))) return -1;

  uint32_t checksum=0;
  const uint16_t *field =  (uint16_t *)current->data;
  for(size_t ihl = current->pos/2; ihl>0;ihl--){
      checksum += *field;
      field++;
  }
  while(checksum >> 16){
    checksum = (checksum & 0xFFFF)+ (checksum>>16);
  }
  *out_checksum = ~((uint16_t)checksum);
  return 0;
}
  
#include "proto/net.nail.cc"
struct ethernet *p_ethernet(NailArena *arena, const uint8_t *buf, size_t nread)
{
   NailMemStream packetstream(buf, nread);
   return parse_ethernet(arena,&packetstream);
}
