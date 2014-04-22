
CFLAGS+=  -ggdb  -std=gnu99
all: extract

extract:  extract.o zip.nail.o zip.o

extract.o: zip.nail.h


zip.nail.h zip.nail.c: zip.nail
	../../generator/nail $< 
	astyle zip.nail.h
	astyle zip.nail.c
zip.nail: 
	true


