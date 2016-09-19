#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>     /* basename() */
#include "id3v1.h"

static void usage(char *argv[]);

int main(int argc, char *argv[])
{
    int n;
    struct id3v1tag tag;
    FILE *mp3;

    if (argc < 2)
        usage(argv);

    for (n=1; n < argc; ++n)
    {
        if ((mp3=fopen(argv[n], "r")) == NULL) {
            fprintf(stderr, "cannot open \"%s\", skipping.\n", argv[n]);
            continue;
        }

        if (get_id3v1_tag(mp3, &tag) == NOTAG) {
            fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            fclose(mp3);
            continue;
        } else {
            printf("FILE\t%s\n", argv[n]);
            print_id3v1_tag(&tag);
            printf("\n");
            fclose(mp3);
        }
    }

    return 0;
}


static void usage(char *argv[])
{
    fprintf(stderr, "usage: %s <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "read id3v1.x mp3 tag info.\n");
    exit(EXIT_FAILURE);
}
