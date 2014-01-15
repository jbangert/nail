generator: foo.c
	cc $(CFLAGS) -o $@ $<
foo.c: *.h ../include/nail/macros/*.h generator.c
	cpp $(CFLAGS) generator.c |sed '/^\#/d' > foo.c && astyle foo.c
CFLAGS+= -I../include -lhammer -ggdb 
