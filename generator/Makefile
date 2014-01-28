all: nail
nail: main.o grammar.o mk_gen.o  expr.o


foo.cxx: mk_parser.cxx *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > foo.c && astyle foo.c

CFLAGS+= -I../ -lhammer -ggdb -std=gnu99
