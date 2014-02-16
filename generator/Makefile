all: nail
newgrammar.h: tools/*.c tools/*.h tools/*.y
	$(cd tools; make; make); ./old2new ../new_grammar.nail  2&>/dev/null > ../newgrammar.h
nail: main.o foo.o mk_gen.o mk_parser.o
	g++ $(CFLAGS) -o $@ $^
foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail
CFLAGS+= -I../ -lhammer -ggdb
CXXFLAGS=$(CFLAGS) -std=gnu++11
