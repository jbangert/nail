server: foo.c generator.c
	cc $(CFLAGS) -o $@ $^
foo.c: server.c *.h ../../nail/*.h
	cpp $(CFLAGS) server.c > foo.c && astyle foo.c
generator.c : grammar.h	
	../../generator/nail $< $@ -generator && astyle $@


CFLAGS+= -I../../include -lhammer -ggdb  -std=gnu99
# server: server.c grammar.h
