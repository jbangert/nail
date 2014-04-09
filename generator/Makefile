all: nail 
CFLAGS+= -I../ -lhammer -ggdb -std=gnu++11
CXXFLAGS=$(CFLAGS) -std=gnu++11
mk_parser.o: mk_parser.cc *.h
mk_gen.o: mk_gen.cc *.h
util.o: util.cc *.h
%.c:%.nail 
	make nail && ./nail $< $@ -parser_hammer -generator && cat testharness.c >> $@ && astyle $@  
newgrammar.h: new_grammar.nail
	tools/old2new new_grammar.nail  2&>/dev/null > newgrammar.h
parser_template.o: parser_template.c 
	ld -r -o $@ --format=binary $<
nail: main.o foo.o mk_parser.o mk_gen.cc parser_template.o util.o
	g++ $(CFLAGS) -o $@ $^


foo.c: grammar.c newgrammar.h *.h ../nail/macros/*.h 
	cpp $(CFLAGS) $< |sed '/^\#/d' > $@ && astyle $@
clean: 
	rm *.o nail

