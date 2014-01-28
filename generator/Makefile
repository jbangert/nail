all: nail
nail: main.o grammar.o mk_gen.o
	cc $(CFLAGS) -o $@ $^
foo.c: mk_parser.c *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > foo.c && astyle foo.c

CFLAGS+= -I../ -lhammer -ggdb -std=gnu99
