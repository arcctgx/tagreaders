CC = gcc
CCOPTS = -Wall -Werror -ansi -pedantic

LTAGLIB = -ltag_c -lstdc++
ITAGLIB = -I/usr/include/taglib/

.PHONY: all clean

all: tagread-v1 tagread csvscrob


id3v1.o: id3v1.c id3v1.h
	$(CC) $(CCOPTS) -c id3v1.c

tagread-v1: tagread-v1.c id3v1.h id3v1.o
	$(CC) $(CCOPTS) tagread-v1.c id3v1.o -o tagread-v1

tagread: tagread.c
	$(CC) $(CCOPTS) tagread.c $(LTAGLIB) $(ITAGLIB) -o tagread

csvscrob: csvscrob.c
	$(CC) $(CCOPTS) csvscrob.c $(LTAGLIB) $(ITAGLIB) -o csvscrob


clean:
	rm -f *.o
	rm -f tagread tagread-v1 csvscrob
