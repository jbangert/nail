struct dnspacket *parse_dnspacket(NailArena *arena,
  NailStream *in);
int gen_dnspacket(NailArena *tmp_arena,
  NailStream *out,
  struct dnspacket *val);
