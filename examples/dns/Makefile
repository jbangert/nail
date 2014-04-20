
CFLAGS+= -I../../include -lhammer -ggdb  -std=gnu99
all: server resolver

server:  resolver.o dns.nail.o dns.o
resolver: resolver.o dns.nail.o dns.o



dns.nail.h dns.nail.c: dns.nail
	../../generator/nail $< 
	astyle $@



# server: server.c grammar.h
