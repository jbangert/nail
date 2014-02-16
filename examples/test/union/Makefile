test: foo.c  
	gcc -lhammer -ggdb -o $@ $< -I../../
foo.c: test.c *.h 
	cpp -I../../../ test.c |sed '/^\#/d' > foo.c && astyle foo.c
