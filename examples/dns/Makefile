server: foo.c
	cc $(CFLAGS) -o $@ $<
foo.c: server.c *.h ../../include/nail/*.h
	cpp $(CFLAGS) server.c |sed '/^\#/d' > foo.c && astyle foo.c


CFLAGS+= -I../../include -lhammer -ggdb 
# server: server.c grammar.h
