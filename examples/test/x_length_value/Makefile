test: foo.c  
	gcc -lhammer -ggdb -o $@ $< -I../../include
foo.c: test.c *.h ../../../include/nail/*.h ../../../include/nail/macros/*.h
	cpp -I../../../include test.c |sed '/^\#/d' > foo.c && astyle foo.c
