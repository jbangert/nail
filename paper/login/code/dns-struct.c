struct dnspacket {
  uint16_t id;
  uint8_t qr;
  /* ... */
  struct {
    struct question *elem;
    size_t count;
  } questions;
};
