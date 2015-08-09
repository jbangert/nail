CFILES=net.nail.c tunclient.c
HEADERS=net.nail.h
CFLAGS+= -std=gnu99  -lhammer -ggdb
tun: $(CFILES) $(HEADERS)
	gcc $(CFLAGS) -o $@ $(CFILES)
net.nail: 
	true
net.nail.h net.nail.c: net.nail
	../../generator/cnail $<  && astyle net.nail.h && astyle net.nail.c
