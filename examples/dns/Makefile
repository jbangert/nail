
CFLAGS+= -I../../include  -ggdb  -std=gnu99
all: server   resolver

server:  server.o dns.nail.o dns.o
resolver: resolver.o dns.nail.o dns.o

server.o: dns.nail.h
resolver.o: dns.nail.h


dns.nail.h dns.nail.c: dns.nail
	../../generator/nail $< 
	astyle dns.nail.h
	astyle dns.nail.c
dns.nail: 
	true


# server: server.c grammar.h
