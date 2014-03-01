all: server
server: server.c generated.h
	cc $(CFLAGS) -o $@ $<
generated.h: dns_packet.nail
	../../generator/nail $< $@ -parser_hammer -generator && astyle $@


CFLAGS+= -I../../include -lhammer -ggdb  -std=gnu99
# server: server.c grammar.h
