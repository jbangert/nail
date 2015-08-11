# Copyright (C) 2000, 2001  Nominum, Inc.  All Rights Reserved.

CC = gcc
LIBOBJS = 
LIBS = -lnsl -lresolv 
DEFS = -DHAVE_CONFIG_H

queryperf: queryperf.o $(LIBOBJS)
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) queryperf.o $(LIBOBJS) $(LIBS) -lm -o queryperf

queryperf.o: queryperf.c
	$(CC) $(CFLAGS) $(DEFS) -c queryperf.c

# under missing subdir
getaddrinfo.o:	./missing/getaddrinfo.c
	$(CC) $(CFLAGS) -c ./missing/$*.c

getnameinfo.o:	./missing/getnameinfo.c
	$(CC) $(CFLAGS) -c ./missing/$*.c

clean:
	rm -f *.o queryperf

distclean: clean
	rm -f config.log
	rm -f config.cache
	rm -f config.status
	rm -f Makefile
