all: test
clean:
	-rm -f test.nail.* test 
test.nail.cc test.nail.hh: test.nail
	../../generator/nail test.nail
	astyle test.nail.hh
	astyle test.nail.cc			
test: test.nail.cc harness.cc
	clang++ -I../../include -ggdb -o $@ harness.cc

