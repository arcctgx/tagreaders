CC := gcc
CCOPTS := -Wall -Werror -ansi -pedantic

LTAGLIB := -ltag_c -lstdc++
ITAGLIB := -I/usr/include/taglib/

.PHONY: all clean

all: id3v1read tagread csvscrob

id3v1.o: id3v1.c id3v1.h
	$(CC) $(CCOPTS) -c $<

id3v1read: id3v1read.c id3v1.h id3v1.o
	$(CC) $(CCOPTS) $< id3v1.o -o $@

tagread: tagread.c
	$(CC) $(CCOPTS) $< $(LTAGLIB) $(ITAGLIB) -o $@

csvscrob: csvscrob.c
	$(CC) $(CCOPTS) $< $(LTAGLIB) $(ITAGLIB) -o $@

clean:
	$(RM) *.o id3v1read tagread csvscrob
