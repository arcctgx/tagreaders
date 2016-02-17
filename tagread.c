/*
 * gcc tagread.c -Wall -ansi -pedantic -I/usr/include/taglib/ -ltag_c -lstdc++
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h> /* for getopt() */
#include <libgen.h> /* for basename() */
#include <stdlib.h> /* for div() */
#include <time.h>
#include <tag_c.h>

#define MAXTIMEBUF 64

int usage(char *argv[])
{
    fprintf(stderr, "usage: %s [OPTIONS] <file> [file2 ...]\n", basename(argv[0]));
    fprintf(stderr, "   -l: enable list output\n");
    fprintf(stderr, "   -c: enable csv output\n");
    fprintf(stderr, "   -t: show total time\n");
    fprintf(stderr, "   -T: enable timestamps in csv output\n");
    fprintf(stderr, "   -q: suppress error messages\n");
    return 1;
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

void get_scrobble_time(time_t ctime, int ttime, char *bufptr, size_t maxbuf)
{
    time_t stime = -1;
    struct tm *tm;

    memset(bufptr, 0, maxbuf);

    /* if length between 30 sec and 8 min, then scrobble at half of track */
    if (ttime >= 30 && ttime < 480) {
        stime = ctime + ttime/2;
    }
    /* if track is longer than 8 min scrobble at 4 min */
    else if (ttime >= 480) {
        stime = ctime + 240;
    }

    if (stime >= 0) {
        tm = localtime(&stime);
        strftime(bufptr, maxbuf, "%Y-%m-%d %H:%M:%S", tm);
    }

    /* tracks shorter than 30s will be silently discarded */

    return;
}


int main(int argc, char *argv[])
{
    char opt;
    extern int optind, opterr;
    int list_mode = 0, csv_mode = 0, show_total = 0, quiet_mode = 0, enable_timestamp = 0;
    int seconds = 0, total = 0;
    int n, bitrate;
    div_t length;
    char *timestamp;
    time_t ctime;
    struct tm tm;
    char timebuf[MAXTIMEBUF] = {0};
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *prop;

    opterr = 0;
    while ((opt = getopt(argc, argv, "lctqT:")) != -1) {
        switch (opt) {
            case 'l':
                list_mode = 1;
                csv_mode = 0;
                break;
            case 'c':
                list_mode = 0;
                csv_mode = 1;
                break;
            case 't':
                show_total = 1;
                break;
            case 'q':
                quiet_mode = 1;
                break;
            case 'T':
                enable_timestamp = 1;
                timestamp = optarg;
                break;
            default:
                break;  /* quietly ignore unknown options */
        }
    }

    if (optind == argc) {   /* no non-option arguments given */
        usage(argv);
    }

    if (enable_timestamp == 1) {
        strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm);
        ctime = mktime(&tm);
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

            bitrate = taglib_audioproperties_bitrate(prop);
            seconds = taglib_audioproperties_length(prop);
            length = div(seconds, 60);
            total += seconds;

            if (list_mode == 1) {
                printf( "%s\t%s\t%4d\t%2d\t%2d:%02d\t%3d kbps\t%s\n", 
                    taglib_tag_artist(tag),
                    taglib_tag_album(tag),
                    taglib_tag_year(tag), 
                    taglib_tag_track(tag),
                    length.quot,    /* minutes */
                    length.rem,     /* seconds */
                    bitrate,
                    taglib_tag_title(tag) );
            } else if (csv_mode == 1) {
                printf( "\"%s\", \"%s\", \"%s\", ",
                    taglib_tag_artist(tag),
                    taglib_tag_title(tag),
                    taglib_tag_album(tag) );

                if (enable_timestamp == 1) {
                    get_scrobble_time(ctime, seconds, timebuf, MAXTIMEBUF);
                    ctime += seconds;
                }

                printf( "\"%s\", \"\", \"%d\"\n", timebuf, seconds);
            } else {
                printf( "FILE\t%s\n", argv[n] );
                if (strlen(taglib_tag_title(tag)) > 0)      printf( "TITLE\t%s\n", taglib_tag_title(tag) );
                if (strlen(taglib_tag_artist(tag)) > 0)     printf( "ARTIST\t%s\n", taglib_tag_artist(tag) );
                if (strlen(taglib_tag_album(tag)) > 0)      printf( "ALBUM\t%s\n", taglib_tag_album(tag) );
                if (taglib_tag_year(tag) != 0)              printf( "YEAR\t%d\n", taglib_tag_year(tag) );
                if (taglib_tag_track(tag) != 0)             printf( "TRACK\t%d\n", taglib_tag_track(tag) );
                if (strlen(taglib_tag_genre(tag)) > 0)      printf( "GENRE\t%s\n", taglib_tag_genre(tag) );
                if (strlen(taglib_tag_comment(tag)) > 0)    printf( "COMMENT\t%s\n", taglib_tag_comment(tag) );
                printf( "LENGTH\t%d:%02d\n", length.quot, length.rem );
                printf( "BITRATE\t%d kbps\n\n", bitrate );
            }
        } else {
            if (quiet_mode == 0) {
                fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            }
            continue;
        }

        taglib_tag_free_strings();
        taglib_file_free(file);
    }

    if (show_total == 1) {
        print_total_time(total);
    }

    return 0;
}
