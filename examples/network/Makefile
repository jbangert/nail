CFILES=net.nail.c tunclient.c
HEADERS=net.nail.h
CFLAGS+= -std=c99  -lhammer -ggdb
tun: $(CFILES) $(HEADERS)
	gcc $(CFLAGS) -o $@ $(CFILES)
net.nail.h net.nail.c: net.nail
	../../generator/nail $<  && astyle net.nail.h && astyle net.nail.c
