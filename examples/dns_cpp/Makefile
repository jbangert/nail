CXX=clang++
CFLAGS+= -I../../include  -std=gnu99 -O3
CXXFLAGS+= -I../../include  -std=gnu++11 -fPIC -O3
all: server  # resolver

server.o:server.cc dns.cc dns.nail.cc
server:  server.o  
resolver: resolver.o  

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
