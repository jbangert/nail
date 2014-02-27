all: nail parser_invocation
newgrammar.h: new_grammar.nail
	tools/old2new new_grammar.nail  2&>/dev/null > newgrammar.h
parser_template.o: parser_template.c
	ld -r -o $@ --format=binary $<
nail: main.o foo.o mk_parser.o parser_template.o
	g++ $(CFLAGS) -o $@ $^
parser_invocation: foo.o parser_invocation.o parser_template.o
	g++ $(CFLAGS) -o $@ $^

foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail
CFLAGS+= -I../ -lhammer -ggdb
CXXFLAGS=$(CFLAGS) -std=gnu++11
