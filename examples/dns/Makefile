
CFLAGS+= -I../../include  -std=gnu99  -O3 
all: server  # resolver

server:  server.o dns.nail.o dns.o
resolver: resolver.o dns.nail.o dns.o

server.o: dns.nail.h
resolver.o: dns.nail.h

clean:
	rm dns.nail.* *.o
dns.nail.h dns.nail.c: dns.nail
	../../generator/cnail $< 
	astyle dns.nail.h
	astyle dns.nail.c
dns.nail: 
	true


# server: server.c grammar.h
