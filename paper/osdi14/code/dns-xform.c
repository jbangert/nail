
int dnscompress_parse(NailArena *tmp,
                  NailStream *out_decomp,
                  NailStream *in_current);

int dnscompress_generate(NailArena *tmp,
                     NailStream *in_decomp,
                     NailStream *out_current);

