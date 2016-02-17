#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h> /* for getopt() */
#include <libgen.h> /* for basename() */
#include <time.h>
#include <tag_c.h>

#define MAXTIMEBUF 64

int usage(char *argv[])
{
    fprintf(stderr, "usage: %s [OPTIONS] <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "   -t <TIMESTAMP>: enable time stamps (format: YYYY-MM-DD hh:mm:ss)\n");
    fprintf(stderr, "   -q: suppress error messages\n");
    return 1;
}


void get_scrobble_time(time_t current_time, int track_time, char *bufptr, size_t maxbuf)
{
    time_t scrobble_time = -1;
    struct tm *tm;

    memset(bufptr, 0, maxbuf);

    /* if track length is between 30 sec and 8 min, scrobble at half of track */
    if (track_time >= 30 && track_time < 480) {
        scrobble_time = current_time + track_time/2;
    }
    /* if track is longer than 8 min scrobble at 4 min */
    else if (track_time >= 480) {
        scrobble_time = current_time + 240;
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
    int quiet_mode = 0, enable_timestamp = 0;
    int n, seconds = 0;
    time_t current_time;
    struct tm tm;
    char timebuf[MAXTIMEBUF] = {0};
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *prop;

    opterr = 0;
    while ((opt = getopt(argc, argv, "qt:")) != -1) {
        switch (opt) {
            case 'q':
                quiet_mode = 1;
                break;
            case 't':
                enable_timestamp = 1;
                strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm);
                current_time = mktime(&tm);
                break;
            default:
                break;  /* quietly ignore unknown options */
        }
    }

    if (optind == argc) {   /* no non-option arguments given */
        usage(argv);
    }

    /* try to avoid mojibake: enable UTF-8 output */
    taglib_set_strings_unicode(1);

    for (n=optind; n < argc; n++) {
        file = taglib_file_new(argv[n]);

        if (file == NULL) {
            if (quiet_mode == 0) {
                fprintf(stderr, "cannot open \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        tag = taglib_file_tag(file);
        prop = taglib_file_audioproperties(file);

        if (tag != NULL && prop != NULL) {
            seconds = taglib_audioproperties_length(prop);

            /* tracks shorter than 30s should be discarded */
            if (seconds < 30) {
                if (quiet_mode == 0) {
                    fprintf(stderr, "track too short (%ds), skipping.\n", seconds);
                }
                continue;
            }

            printf( "\"%s\", \"%s\", \"%s\", ",
                taglib_tag_artist(tag),
                taglib_tag_title(tag),
                taglib_tag_album(tag) );

            if (enable_timestamp == 1) {
                get_scrobble_time(current_time, seconds, timebuf, MAXTIMEBUF);
                current_time += seconds;
            }

            printf( "\"%s\", \"\", \"%d\"\n", timebuf, seconds);
        } else {
            if (quiet_mode == 0) {
                fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        taglib_tag_free_strings();
        taglib_file_free(file);
    }

    return 0;
}
