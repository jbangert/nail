all: nail 

CFLAGS+= -I../ -lhammer -ggdb -std=gnu++11 -Wno-enum-compare

.SUFFIXES:

CXXFLAGS=$(CFLAGS) -std=gnu++11
util.o: util.cc *.h
%.c:%.nail 
	make nail && ./nail $< $@ -parser_hammer -generator && cat testharness.c >> $@ && astyle $@  
new_grammar.nail.c new_grammar.nail.h: new_grammar.nail
	tools/old2new new_grammar.nail  2&>/dev/null > newgrammar.h
parser_template.o: parser_template.c 
	ld -r -o $@ --format=binary $<

nail: main.cc mk_parser.cc mk_gen.cc grammar.c new_grammar.nail.c
	clang++ $(CFLAGS) -o $@ $^


foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail

