
CFLAGS+= -I../../include  -std=gnu99 -O3 # -ggdb 
CXXFLAGS+= -I../../include  -std=gnu99 -O3 # -ggdb 
all: server  # resolver

dns.o: dns.cc dns.nail.cc
server:  server.o  dns.o
resolver: resolver.o  dns.o

server.o: dns.nail.hh
resolver.o: dns.nail.hh


clean:
	rm dns.nail.* *.o
dns.nail.hh dns.nail.cc: dns.nail
	../../generator/nail $< 
	astyle dns.nail.hh
	astyle dns.nail.cc
dns.nail: 
	true


# server: server.c grammar.h
