
all: dep parse
include common.mk
dep:
	make -C proto
parse:  parse.cc proto/net.nail.*
clean:
	make -C proto clean
