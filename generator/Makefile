all: nail
nail: main.o foo.o mk_gen.o
	g++ $(CFLAGS) -o $@ $^
foo.c: grammar.c *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o *.orig
CFLAGS+= -I../ -lhammer -ggdb
