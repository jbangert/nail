server: foo.c  
	gcc -lhammer -ggdb -std=c99 -o $@ $< -I../../include
foo.c: server.c *.h ../../include/hammer/*.h
	cpp -I../../include server.c |sed '/^\#/d' > foo.c && astyle foo.c


# CFLAGS+= -I../../include -lhammer -ggdb 
# server: server.c grammar.h
