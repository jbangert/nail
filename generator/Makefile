COMFLAGS := -I.. -ggdb -Wno-enum-compare
CFLAGS	 := $(COMFLAGS) -std=gnu11
CXXFLAGS := $(COMFLAGS) -std=c++11 -Wno-format -Wno-switch
LDFLAGS	 := -lhammer

.SUFFIXES:

all: nail 

util.o: util.cc *.h
%.c: %.nail nail
	./nail $< $@ -parser_hammer -generator
	cat testharness.c >> $@
	astyle $@  

# %.nail.c %.nail.h: %.nail nail
regen-nail-grammar: new_grammar.nail nail
	./nail $<
	astyle $<.c
	astyle $<.h

parser_template.o: parser_template.c 
	ld -r -o $@ --format=binary $<

nail: main.o mk_parser.o mk_gen.o util.o new_grammar.nail.o parser_template.o
	clang++ $^ -o $@ $(LDFLAGS)

%.o: %.cc $(wildcard *.h)
	clang++ $(CXXFLAGS) -c $< -o $@

%.o: %.c $(wildcard *.h)
	clang $(CFLAGS) -c $< -o $@

foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@

clean: 
	rm -f *.o nail

