all: generator parser_invocation
parser_invocation: parser_invocation.c
	cc $(CFLAGS) -o $@ $<
generator: foo.c
	cc $(CFLAGS) -o $@ generator.c
foo.c: *.h ../include/nail/macros/*.h generator.c
	cpp $(CFLAGS) generator.c |sed '/^\#/d' > foo.c && astyle foo.c
CFLAGS+= -I../include -lhammer -ggdb -std=gnu99
