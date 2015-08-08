
all: dep parse
include common.mk
dep:
	make -C proto
parse:  parse.cc
clean:
	make -C proto clean
