#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* for getopt() */
#include <libgen.h> /* for basename() */
#include <time.h>
#include <tag_c.h>

#define YES 1
#define NO 0
#define MAXTIMEBUF 64
#define SHORT_TRACK_LENGTH (30)     /* seconds */
#define LONG_TRACK_LENGTH (480)     /* seconds */


static void usage(char *argv[])
{
    fprintf(stderr, "usage: %s [OPTIONS] <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "   -t <[YYYY-MM-DD ]hh:mm:ss> | <now>: specify timestamp of beginning of first track\n");
    fprintf(stderr, "   -s: enable scrobbling of tracks shorter than %d seconds\n", SHORT_TRACK_LENGTH);
    fprintf(stderr, "   -q: suppress error messages\n");
    fprintf(stderr, "   -U: disable UTF-8 output\n");
    exit(EXIT_FAILURE);
}


static time_t init_current_time(char *str)
{
    time_t current_time;
    struct tm tm;

    if (strncmp(str, "now", 3) == 0) {
        return time(NULL);
    } else {
        memset(&tm, 0, sizeof(struct tm));
        /* try to parse full date first */
        if (strptime(str, "%Y-%m-%d %H:%M:%S", &tm) == NULL) {
            /* if it fails assume current day, only attempt to parse hour */
            current_time = time(NULL);
            tm = *localtime(&current_time);
            if (strptime(str, "%H:%M:%S", &tm) == NULL) {
                /* if it fails don't use timestamps at all */
                return -1;
            }
        }
        tm.tm_isdst = -1;   /* automatically determine if DST is in effect */
        return mktime(&tm);
    }
}


static void get_scrobble_time(time_t current_time, int track_time, char *bufptr, size_t maxbuf)
{
    time_t scrobble_time = -1;
    struct tm *tm;

    memset(bufptr, 0, maxbuf);

    /* if track length is less than 8 min, scrobble at half of track */
    if (track_time < LONG_TRACK_LENGTH) {
        scrobble_time = current_time + track_time/2;
    }
    /* if track is longer than 8 min scrobble at 4 min */
    else if (track_time >= LONG_TRACK_LENGTH) {
        scrobble_time = current_time + LONG_TRACK_LENGTH/2;
    }

    if (scrobble_time >= 0) {
        tm = localtime(&scrobble_time);
        strftime(bufptr, maxbuf, "%Y-%m-%d %H:%M:%S", tm);
    }

    return;
}


int main(int argc, char *argv[])
{
    char opt;
    extern int optind, opterr;
    int verbose_mode = YES, timestamps_enabled = NO, short_tracks_enabled = NO;
    int n, track_seconds = 0;
    time_t current_time;
    char timebuf[MAXTIMEBUF] = {0};
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *prop;

    opterr = 0;
    while ((opt = getopt(argc, argv, "t:sqU")) != -1) {
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
            case 'U':
                taglib_set_strings_unicode(NO);
                break;
            default:
                fprintf(stderr, "option -%c is not supported!\n", optopt);
                break;
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

        tag = taglib_file_tag(file);
        prop = taglib_file_audioproperties(file);

        if (tag == NULL || prop == NULL) {
            if (verbose_mode == YES) {
                fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            }
            taglib_file_free(file);
            continue;
        }

        track_seconds = taglib_audioproperties_length(prop);

        /* discard track shorter than 30 seconds by default */
        if (short_tracks_enabled == NO) {
            if (track_seconds < SHORT_TRACK_LENGTH) {
                if (verbose_mode == YES) {
                    fprintf(stderr, "track too short (%ds), skipping.\n", track_seconds);
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
            get_scrobble_time(current_time, track_seconds, timebuf, MAXTIMEBUF);
            current_time += track_seconds;
        }

        printf( "\"%s\", \"\", \"%d\"\n", timebuf, track_seconds);

        taglib_tag_free_strings();
        taglib_file_free(file);
    }

    return 0;
}
