all: generator parser_invocation
parser_invocation: parser_invocation.c
	cc $(CFLAGS) -o $@ $<
generator: foo.c
	cc $(CFLAGS) -o $@ generator.c
foo.c: *.h ../nail/macros/*.h generator.c
	cpp $(CFLAGS) generator.c |sed '/^\#/d' > foo.c && astyle foo.c
CFLAGS+= -I../ -lhammer -ggdb -std=gnu99
