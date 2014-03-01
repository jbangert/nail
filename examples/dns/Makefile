all: server
server: server.c parser.h
	cc $(CFLAGS) -o $@ $<
%.h: %.nail
	../../generator/nail $< $@ -parser_hammer -generator && astyle $@


CFLAGS+= -I../../include -lhammer -ggdb  -std=gnu99
# server: server.c grammar.h
