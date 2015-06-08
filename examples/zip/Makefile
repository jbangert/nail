
CFLAGS+=  -ggdb  -std=gnu99
LDFLAGS+= -lz
all: extract

extract:  extract.o zip.nail.o zip.o

extract.o: zip.nail.h


zip.nail.h zip.nail.c: zip.nail
	../../generator/cnail $< 
	astyle zip.nail.h
	astyle zip.nail.c
zip.nail: 
	true


