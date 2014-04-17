all: nail 

CFLAGS+= -I../ -lhammer -ggdb -std=gnu++11 -Wno-enum-compare

.SUFFIXES:

CXXFLAGS=$(CFLAGS) -Wno-format -Wno-switch
util.o: util.cc *.h
%.c:%.nail 
	make nail && ./nail $< $@ -parser_hammer -generator && cat testharness.c >> $@ && astyle $@  
new_grammar.nail.c new_grammar.nail.h: new_grammar.nail
	./nail new_grammar.nail && astyle new_grammar.nail.c && astyle new_grammar.nail.h
parser_template.c.o: parser_template.c 
	ld -r -o $@ --format=binary $<
parser_template.h.o : parser_template.h
	ld -r -o $@ --format=binary $<

nail: main.cc mk_parser.cc mk_gen.cc util.cc new_grammar.nail.c parser_template.c.o parser_template.h.o
	clang++ $(CXXFLAGS) -o $@ $^


foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail

