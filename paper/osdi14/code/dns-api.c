struct dnspacket*parse_dnspacket(NailArena *arena, 
  const uint8_t *data, 
  size_t size);
int gen_dnspacket(NailArena *tmp_arena,
  NailStream *out,
  struct dnspacket * val) ;
