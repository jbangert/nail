all: server
server: foo.c generator.c
	cc $(CFLAGS) -o $@ $^
foo.c: server.c *.h ../../nail/*.h
	cpp $(CFLAGS) server.c |sed '/^\#/d' > $@ && astyle $@ && rm $@.orig
generator.c : grammar.h *.h	
	../../generator/nail $< $@ -generator && astyle $@


CFLAGS+= -I../../include -lhammer -ggdb  -std=gnu99
# server: server.c grammar.h
