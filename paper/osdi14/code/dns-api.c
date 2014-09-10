struct dnspacket {
    uint16_t id;
    uint8_t qr;
    /*  abbreviated */
    struct {
        question*elem;
        size_t count;
    } questions;
}; 
/* Other structures */
struct dnspacket*parse_dnspacket(
  NailArena *arena, 
  const uint8_t *data, 
  size_t size);
int gen_dnspacket(NailArena *tmp_arena,
  NailStream *out,
  struct dnspacket * val) ;
