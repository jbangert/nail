CXXFLAGS+=-ggdb
EXECS=tunclient
all: dep $(EXECS)
include common.mk
dep:
	make -C proto
tunclient:  parse.cc tunclient.cc
clean:
	make -C proto clean && rm $(EXECS) || true
