all: nail
nail: main.o foo.o mk_gen.o mk_parser.o
	g++ $(CFLAGS) -o $@ $^
foo.c: grammar.c *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail
CFLAGS+= -I../ -lhammer -ggdb
CXXFLAGS=$(CFLAGS) -std=gnu++11
