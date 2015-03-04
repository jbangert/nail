#include "test.nail.hh"
#include "nail/memstream.hh"
#include "test.nail.cc"
#include <cstdio>
int main(){
  uint8_t packet[4096*1024]; // static buffer for simplicity
  size_t remote_len = fread(packet,1,sizeof packet,stdin);
  NailArena arena;
  NailArena_init(&arena,4096);
  NailMemStream stream(packet, remote_len);
  foo *f = parse_foo(&arena, &stream);
  if(!f){
    printf("not parsed");
    exit(-1);
  } else {
    printf("parsed");
    exit(0);
  }
}
