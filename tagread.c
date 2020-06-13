#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for getopt() */
#include <libgen.h> /* for basename() */
#include <stdlib.h> /* for div() */
#include <tag_c.h>
#include "albumartist_c.h"

#define YES 1
#define NO 0

void usage(char *argv[])
{
    fprintf(stderr, "usage: %s [OPTIONS] <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "   -l: enable list output\n");
    fprintf(stderr, "   -t: show total time\n");
    fprintf(stderr, "   -q: suppress error messages\n");
    exit(EXIT_FAILURE);
}


void print_total_time(int seconds)
{
    int hour = 0, min = 0, sec = 0;
    div_t d;

    d = div(seconds, 60);
    sec = d.rem;
    min = d.quot;

    if (min < 60) {
        printf("total time: %2d:%02d\n", min, sec);
    } else {
        d = div(min, 60);
        hour = d.quot;
        min = d.rem;
        printf("total time: %d:%02d:%02d\n", hour, min, sec);
    }

    return;
}


int main(int argc, char *argv[])
{
    char opt;
    extern int optind, opterr;
    int list_mode = NO, show_total = NO, verbose_mode = YES;
    int seconds = 0, total = 0;
    int nfiles = 0;
    int n, bitrate;
    div_t length;
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *prop;
    char *album_artist = NULL;

    opterr = 0;
    while ((opt = getopt(argc, argv, "ltq")) != -1) {
        switch (opt) {
            case 'l':
                list_mode = YES;
                break;
            case 't':
                show_total = YES;
                break;
            case 'q':
                verbose_mode = NO;
                break;
            default:
                break;  /* quietly ignore unknown options */
        }
    }

    if (optind == argc) {   /* no non-option arguments given */
        usage(argv);
    }

    for (n=optind; n < argc; n++) {
        file = taglib_file_new(argv[n]);

        if (file == NULL) {
            if (verbose_mode == YES) {
                fprintf(stderr, "cannot open \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        ++nfiles;
        tag = taglib_file_tag(file);
        prop = taglib_file_audioproperties(file);

        if (tag != NULL && prop != NULL) {
            album_artist = get_album_artist(file);
            bitrate = taglib_audioproperties_bitrate(prop);
            seconds = taglib_audioproperties_length(prop);
            length = div(seconds, 60);
            total += seconds;

            if (list_mode == YES) {
                printf( "%s\t%s\t%s\t%4d\t%2d\t%2d:%02d\t%3d kbps\t%s\n",
                    taglib_tag_artist(tag),
                    taglib_tag_album(tag),
                    album_artist[0] == '\0' ? "\b" : album_artist,  /* delete preceding tab if album artist not set */
                    taglib_tag_year(tag),
                    taglib_tag_track(tag),
                    length.quot,    /* minutes */
                    length.rem,     /* seconds */
                    bitrate,
                    taglib_tag_title(tag) );
            } else {
                printf( "FILE\t%s\n", argv[n] );
                if (strlen(taglib_tag_title(tag)) > 0)      printf( "TITLE\t%s\n", taglib_tag_title(tag) );
                if (strlen(taglib_tag_artist(tag)) > 0)     printf( "ARTIST\t%s\n", taglib_tag_artist(tag) );
                if (strlen(taglib_tag_album(tag)) > 0)      printf( "ALBUM\t%s\n", taglib_tag_album(tag) );
                if (strlen(album_artist) > 0)               printf( "ALBART\t%s\n", album_artist );
                if (taglib_tag_year(tag) != 0)              printf( "YEAR\t%d\n", taglib_tag_year(tag) );
                if (taglib_tag_track(tag) != 0)             printf( "TRACK\t%d\n", taglib_tag_track(tag) );
                if (strlen(taglib_tag_genre(tag)) > 0)      printf( "GENRE\t%s\n", taglib_tag_genre(tag) );
                if (strlen(taglib_tag_comment(tag)) > 0)    printf( "COMMENT\t%s\n", taglib_tag_comment(tag) );
                printf( "LENGTH\t%d:%02d\n", length.quot, length.rem );
                printf( "BITRATE\t%d kbps\n\n", bitrate );
            }
        } else {
            if (verbose_mode == YES) {
                fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        free(album_artist);
        taglib_tag_free_strings();
        taglib_file_free(file);
    }

    if (nfiles != 0 && show_total == YES) {
        print_total_time(total);
    }

    return 0;
}
