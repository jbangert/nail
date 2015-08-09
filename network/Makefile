
all: dep parse
include common.mk
dep:
	make -C proto
parse:  parse.cc tunclient.cc
clean:
	make -C proto clean
