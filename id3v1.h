#ifndef _ID3V1_H_
#define _ID3V1_H_

#include <stdio.h>
#include <string.h>
#include <libgen.h> /* basename() */
#include <ctype.h>

#define ID3SIZE (128)   /* id3v1.0/1.1 tag has a fixed size of 128 bytes */
#define NOTAG (0)
#define ID3V10 (10)
#define ID3V11 (11)


struct id3v1tag {
    int version;
    char title[30+1];
    char artist[30+1];
    char album[30+1];
    char year[4+1];
    char comment[30+1]; /* 30 chars in id3v1.0, 28 chars in id3v1.1 */
    int track;
    int genre;
};


int usage(char *argv[]);

int has_id3v1_tag(FILE *mp3file);

int get_id3v1_tag(FILE *mp3file, struct id3v1tag *tag);

void print_id3v1_tag(struct id3v1tag *tag);

#endif
