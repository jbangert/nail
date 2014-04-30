int
offset_u32_parse(NailArena *tmp,
                 NailStream *out_str,
                 NailStream *in_current,
                 const uint32_t *off)
{
  /* out_str = suffix of in_current
               at offset off */
}

int
offset_u32_generate(NailArena *tmp,
                    NailStream *in_fragment,
                    NailStream *out_current,
                    uint32_t *off)
{
  /* off = position of out_current */
  /* append in_fragment to out_current */
}
