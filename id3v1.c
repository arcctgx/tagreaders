#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "id3v1.h"

/*
 * http://de.wikipedia.org/wiki/Liste_der_ID3v1-Genres
 */
static char *genre_list[] = {
    /* in ID3v1 specification */
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal",
    "New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative",
    "Ska", "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal",
    "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip",
    "Gospel", "Noise", "AlternRock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
    "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
    "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk",
    "Jungle", "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
    "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock",
    /* early Winamp extensions */
    "Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
    "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
    "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson", "Opera", "Chamber Music",
    "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo",
    "A capella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore Techno", "Terror",
    "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal",
    "Crossover", "Contemporary Christian", "Christian Rock",
    /* since Winamp 1.91 */
    "Merengue", "Salsa", "Thrash Metal", "Anime", "Jpop", "Syntpop",
    /* since Winamp 5.6 */
    "Abstract", "Art Rock", "Baroque", "Bhangra", "Big Beat", "Breakbeat", "Chillout", "Downtempo", "Dub",
    "EBM", "Electric", "Electro", "Electroclash", "Emo", "Experimental", "Garage", "Global", "IDM",
    "Illbient", "Industro-Goth", "Jam Band", "Krautrock", "Leftfield", "Lounge", "Math Rock", "New Romantic",
    "Nu-Breakz", "Post-Punk", "Post-Rock", "Psytrance", "Shoegaze", "Space Rock", "Trop Rock", "World Music",
    "Neoclassical", "Audiobook", "Audio Theatre", "Neue Deutsche Welle", "Podcast", "Indie Rock", "G-Funk",
    "Dubstep", "Garage Rock", "Psybient"
    };
static int num_genres = sizeof(genre_list)/sizeof(genre_list[0]);


static void del_trail_white(char *string);
static void sanitize_id3v1_strings(struct id3v1tag *tag);


int has_id3v1_tag(FILE *mp3)
{
    int status = NOTAG;
    long int pos;
    char tagstr[ID3SIZE] = {0};

    pos = ftell(mp3);
    fseek(mp3, -ID3SIZE, SEEK_END);
    fread(tagstr, sizeof(char), ID3SIZE, mp3);
    if (strncmp(tagstr, "TAG", 3) == 0) {  /* id3v1.0 or v1.1 tag exists */
        status = ID3V10;
        if (tagstr[ID3SIZE-3] == '\0')   /* NULL separator is present, indicating id3v1.1 */
            status = ID3V11;
    }

    fseek(mp3, pos, SEEK_SET);
    return status;
}


int get_id3v1_tag(FILE *mp3, struct id3v1tag *tag)
{
    long int pos;
    char tagstr[ID3SIZE] = {0};

    pos = ftell(mp3);

    memset(tag, 0, sizeof(struct id3v1tag));
    tag->version = has_id3v1_tag(mp3);

    if (tag->version != NOTAG) {
        /* load tag contents */
        fseek(mp3, -ID3SIZE, SEEK_END);
        fread(tagstr, sizeof(char), ID3SIZE, mp3);
        fseek(mp3, pos, SEEK_SET);

        /* take care of common fields first */
        strncpy(tag->title, tagstr+3, 30);
        strncpy(tag->artist, tagstr+33, 30);
        strncpy(tag->album, tagstr+63, 30);
        strncpy(tag->year, tagstr+93, 4);
        tag->genre = (unsigned char)tagstr[127];    /* genre is an 8-bit integer */

        /* id3v1.0: 30-char comment, track number unsupported */
        if (tag->version == ID3V10) {
            strncpy(tag->comment, tagstr+97, 30);
            tag->track = -1;
        }
        /* id3v1.1: 28-char comment, track number present */
        else if (tag->version == ID3V11) {
            strncpy(tag->comment, tagstr+97, 28);
            tag->track = (unsigned char)tagstr[126];
        }

        sanitize_id3v1_strings(tag);
    }

    return tag->version;
}


void print_id3v1_tag(struct id3v1tag *tag)
{
    if (tag->version != NOTAG) {
        if (strlen(tag->title) > 0)
            printf("TITLE\t%s\n", tag->title);
        if (strlen(tag->artist) > 0)
            printf("ARTIST\t%s\n", tag->artist);
        if (strlen(tag->album) > 0)
            printf("ALBUM\t%s\n", tag->album);
        if (strlen(tag->year) > 0)
            printf("YEAR\t%s\n", tag->year);
        if (tag->track > 0)
            printf("TRACK\t%d\n", tag->track);
        if (tag->genre >= 0 && tag->genre < num_genres)
            printf("GENRE\t%s\n", genre_list[tag->genre]);
        if (strlen(tag->comment) > 0)
            printf("COMMENT\t%s\n", tag->comment);
    }

    return;
}


static void del_trail_white(char *string)
{
    int n;

    for (n=strlen(string)-1; n >= 0; n--)
        if (isspace(string[n]) == 0) {
            string[n+1] = '\0';
            break;
        }

    if (n == -1)     /* we went through entire string, it's empty */
        string[0] = '\0';

    return;
}


static void sanitize_id3v1_strings(struct id3v1tag *tag)
{
    del_trail_white(tag->title);
    del_trail_white(tag->artist);
    del_trail_white(tag->album);
    del_trail_white(tag->year);
    del_trail_white(tag->comment);

    return;
}
