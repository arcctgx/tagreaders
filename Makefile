CC = gcc
CCOPTS = -Wall -Werror -ansi -pedantic

LTAGLIB = -ltag_c -lstdc++
ITAGLIB = -I /usr/include/taglib/


all: tagread_v1 tagread csvscrob


tagread_v1: tagread_v1.c
	$(CC) $(CCOPTS) tagread_v1.c -o tagread_v1

tagread: tagread.c
	$(CC) $(CCOPTS) tagread.c $(LTAGLIB) $(ITAGLIB) -o tagread

csvscrob: csvscrob.c
	$(CC) $(CCOPTS) csvscrob.c $(LTAGLIB) $(ITAGLIB) -o csvscrob

clean:
	rm -f tagread_v1
	rm -f tagread
	rm -f csvscrob
