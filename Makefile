CCOPTS := -Wall -Werror
CXXOPTS := -Wall -Werror

LTAGLIB := -ltag -ltag_c -lstdc++
ITAGLIB := -I. -I/usr/include/taglib/

ifeq ($(ASAN), yes)
CCOPTS += -fsanitize=address
CXXOPTS += -fsanitize=address
endif

.PHONY: all clean

all: id3v1read tagread csvscrob

id3v1.o: id3v1.c id3v1.h
	$(CC) $(CCOPTS) -c $<

id3v1read: id3v1read.c id3v1.h id3v1.o
	$(CC) $(CCOPTS) $< id3v1.o -o $@

albumartist_c.o: albumartist_c.cpp albumartist_c.h
	$(CXX) $(CXXOPTS) -c $< $(ITAGLIB)

tagread: tagread.c albumartist_c.h albumartist_c.o
	$(CC) $(CCOPTS) $< albumartist_c.o $(LTAGLIB) $(ITAGLIB) -o $@

csvscrob: csvscrob.c albumartist_c.h albumartist_c.o
	$(CC) $(CCOPTS) $< albumartist_c.o $(LTAGLIB) $(ITAGLIB) -o $@

clean:
	$(RM) *.o id3v1read tagread csvscrob
