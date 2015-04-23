COMFLAGS := -I.. -ggdb -Wno-enum-compare
CFLAGS	 := $(COMFLAGS) -std=gnu11
CXXFLAGS := $(COMFLAGS) -std=c++11 -Wno-format -Wno-switch
LDFLAGS	 := # -lhammer

.SUFFIXES:

all: nail 

util.o: util.cc *.h *.hpp
# %.nail.c %.nail.h: %.nail nail
regen-nail-grammar: new_grammar.nail nail
	./cnail $<
	astyle $<.c
	astyle $<.h
parser_template.cc.cc: parser_template.cc
	xxd -i $< > $@
parser_template.c.cc: parser_template.c 
	xxd -i $< > $@
parser_template.h.cc : parser_template.h
	xxd -i $< > $@

nail_wrap.c : new_grammar.nail.h nail.i
	swig -ruby nail.i

rbnail.so: nail_wrap.c
	gcc -c  nail_wrap.c
nail: main.o mk_parser.o mk_gen.o datatype.o util.o new_grammar.nail.o parser_template.c.o mk_directparser.o parser_template.h.o parser_template.cc.o
	clang++ $^ -o $@ $(LDFLAGS)

%.o: %.cc $(wildcard *.h)
	clang++ $(CXXFLAGS) -c $< -o $@

%.o: %.c $(wildcard *.h)
	clang $(CFLAGS) -c $< -o $@


clean: 
	rm -f *.o nail parser_template.cc.cc parser_template.c.cc parser_template.h.cc

