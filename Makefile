CC = gcc
CCOPTS = -Wall -Werror -ansi -pedantic

LTAGLIB = -ltag_c -lstdc++
ITAGLIB = -I /usr/include/taglib/


all: tagread_v1 tagread


tagread_v1: tagread_v1.c
	$(CC) $(CCOPTS) tagread_v1.c -o tagread_v1

tagread: tagread.c
	$(CC) $(CCOPTS) tagread.c $(LTAGLIB) $(ITAGLIB) -o tagread

clean:
	rm -f tagread_v1
	rm -f tagread
