#ifndef PARSE_HH
#define PARSE_HH
#include "proto/net.nail.hh"
#include <nail/memstream.hh>
extern struct ethernet *p_ethernet(NailArena *arena, const uint8_t *buf, size_t nread);
#endif
