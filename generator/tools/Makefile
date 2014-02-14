old2new: old2new.c bootstrap.c main.cc tokennames.cc
	g++ -std=gnu++0x -ggdb -o $@ $^
bootstrap.c: bootstrap.re2c
	re2c -o $@ $<
old2new.c: old2new.y
	lemon  old2new.y 
tokennames.cc: old2new.c
	awk -vOFS='' 'BEGIN{print "const char *tokennames[] = {\"\"," } {print "\"",$$2,"\","} END{print "\"\"};"};' < old2new.h > $@
