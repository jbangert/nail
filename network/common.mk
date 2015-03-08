TOP := $(dir $(lastword $(MAKEFILE_LIST)))
NAILINCLUDE := $(TOP)/../include/
NAILBIN := $(TOP)/../generator/nail
CFLAGS+= -I $(NAILINCLUDE) -std=gnu99
CXXFLAGS+= -I $(NAILINCLUDE) -std=gnu++11
CXX = clang++
_nail:
	make -C $(TOP)/../generator

%.nail.hh %.nail.cc: %.nail
	$(NAILBIN) $<
	astyle $<.hh || true
	astyle $<.cc || true
