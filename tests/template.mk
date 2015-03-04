test: test.nail.hh test.nail.cc harness.cc
	c++ -o $@ harness.cc
test.nail.hh test.nail.cc: test.nail
	../../generator/nail $< 
	astyle test.nail.hh
	astyle test.nail.cc
