test: foo.c  
	gcc -lhammer -ggdb -o $@ $< -I../../include
foo.c: test.c *.h
	cpp -I../../include test.c |sed '/^\#/d' > foo.c && astyle foo.c
