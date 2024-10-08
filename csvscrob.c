#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* getopt() */
#include <libgen.h> /* basename() */
#include <time.h>
#include <tag_c.h>
#include "albumartist_c.h"

#define YES (1)
#define NO  (0)
#define MAXTIMEBUF (64)
#define SHORT_TRACK_LENGTH (30)     /* seconds */


static void usage(char *argv[])
{
    fprintf(stderr, "usage: %s [OPTIONS] <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "   -t <[YYYY-MM-DD ]hh:mm:ss> | <now>: specify timestamp of beginning of first track\n");
    fprintf(stderr, "   -s: allow scrobbling short tracks\n");
    fprintf(stderr, "   -q: suppress error messages\n");
    fprintf(stderr, "   -u: use UTC time in output\n");
    exit(EXIT_FAILURE);
}


static time_t init_current_time(char *timestr)
{
    time_t current_time;
    struct tm tm;

    if (strcmp(timestr, "now") == 0) {
        return time(NULL);
    } else {
        memset(&tm, 0, sizeof(struct tm));
        /* try to parse full date first */
        if (strptime(timestr, "%Y-%m-%d %H:%M:%S", &tm) == NULL) {
            /* if it fails assume current day, only attempt to parse hour */
            current_time = time(NULL);
            tm = *localtime(&current_time);
            if (strptime(timestr, "%H:%M:%S", &tm) == NULL) {
                /* if it fails don't use timestamps at all */
                return -1;
            }
        }
        tm.tm_isdst = -1;   /* automatically determine if DST is in effect */
        return mktime(&tm);
    }
}


int main(int argc, char *argv[])
{
    char opt;
#ifndef __CYGWIN__
    extern int optind, opterr;
#endif
    int verbose_mode = YES, timestamps_enabled = NO, short_tracks_enabled = NO;
    int n, track_seconds = 0;
    time_t current_time;
    char timebuf[MAXTIMEBUF] = {0};
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *prop;
    struct tm* (*timefunc)(const time_t*) = &localtime;
    char *album_artist = NULL;

    opterr = 0;
    while ((opt = getopt(argc, argv, "t:squ")) != -1) {
        switch (opt) {
            case 't':
                timestamps_enabled = YES;
                current_time = init_current_time(optarg);
                break;
            case 's':
                short_tracks_enabled = YES;
                break;
            case 'q':
                verbose_mode = NO;
                break;
            case 'u':
                timefunc = &gmtime;
                break;
            default:
                fprintf(stderr, "option -%c is not supported!\n", optopt);
                break;
        }
    }

    if (optind == argc) {   /* no non-option arguments given */
        usage(argv);
    }

    for (n=optind; n < argc; ++n) {
        file = taglib_file_new(argv[n]);

        if (file == NULL) {
            if (verbose_mode == YES) {
                fprintf(stderr, "cannot open \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        tag = taglib_file_tag(file);
        prop = taglib_file_audioproperties(file);

        if (tag == NULL || prop == NULL) {
            if (verbose_mode == YES) {
                fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            }
            taglib_file_free(file);
            continue;
        }

        album_artist = get_album_artist(file);
        track_seconds = taglib_audioproperties_length(prop);

        /* by default only consider tracks longer than 30 seconds */
        if (short_tracks_enabled == NO) {
            if (track_seconds <= SHORT_TRACK_LENGTH) {
                if (verbose_mode == YES) {
                    fprintf(stderr, "track too short (%d s), skipping.\n", track_seconds);
                }
                current_time += track_seconds;
                taglib_file_free(file);
                continue;
            }
        }

        printf( "\"%s\", \"%s\", \"%s\", ",
            taglib_tag_artist(tag),
            taglib_tag_title(tag),
            taglib_tag_album(tag) );

        if (timestamps_enabled == YES && current_time != -1) {
            strftime(timebuf, MAXTIMEBUF, "%Y-%m-%d %H:%M:%S", timefunc(&current_time));
            current_time += track_seconds;
        }

        printf( "\"%s\", \"%s\", \"%d\"\n", timebuf, album_artist, track_seconds);

        free(album_artist);
        taglib_tag_free_strings();
        taglib_file_free(file);
    }

    return 0;
}
