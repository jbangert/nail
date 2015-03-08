all: _nail net.nail.cc net.nail.hh
include ../common.mk
SRCS=$(filter-out net.nail,$(wildcard *.nail))
net.nail: $(SRCS)
	@	
clean:
	rm  *.nail.cc *.nail.hh
