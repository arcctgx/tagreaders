#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>     /* for basename() */


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
void del_trail_white(char *string);
void sanitize_id3v1_strings(struct id3v1tag *tag);
void print_id3v1_tag(struct id3v1tag *tag);


int main(int argc, char *argv[])
{
    int n;
    struct id3v1tag tag;
    FILE *mp3;

    if (argc < 2)
        usage(argv);

    for (n=1; n < argc; n++)
    {
        if ((mp3=fopen(argv[n], "r")) == NULL) {
            fprintf(stderr, "cannot open \"%s\", skipping.\n", argv[n]);
            continue;
        }

        if (get_id3v1_tag(mp3, &tag) == NOTAG) {
            fprintf(stderr, "cannot read tag data of \"%s\", skipping.\n", argv[n]);
            fclose(mp3);
            continue;
        } else if (tag.version == ID3V10 || tag.version == ID3V11) {
            printf("FILE\t%s\n", argv[n]);
            sanitize_id3v1_strings(&tag);
            print_id3v1_tag(&tag);
            printf("\n");
        }

        fclose(mp3);
    }

    return 0;
}


int usage(char *argv[])
{
    printf("usage: %s <file> [file2 ...]\n", basename(argv[0]));
    printf("read id3v1.x mp3 tag info.\n\n");
    return 1;
}


int has_id3v1_tag(FILE *mp3)
{
    int status = NOTAG;
    long int pos;
    char tag[ID3SIZE];

    pos = ftell(mp3);
    fseek(mp3, -ID3SIZE, SEEK_END);
    fgets(tag, ID3SIZE, mp3);
    if (strncmp(tag, "TAG", 3) == 0) {  /* id3v1.0 or v1.1 tag exists */
        status = ID3V10;
        if (tag[ID3SIZE-3] == '\0')   /* NULL separator is present, indicating id3v1.1 */
            status = ID3V11;
    }

    fseek(mp3, pos, SEEK_SET);
    return status;
}


int get_id3v1_tag(FILE *mp3, struct id3v1tag *tag)
{
    long int pos;
    char tagstr[ID3SIZE+1];  /* +1 because fgets puts NULL at the end of buffer */

    pos = ftell(mp3);

    if ((tag->version=has_id3v1_tag(mp3)) == NOTAG)
        return NOTAG;
    else {
        fseek(mp3, -ID3SIZE, SEEK_END);
        fread(tagstr, sizeof(char), ID3SIZE, mp3);
        fseek(mp3, pos, SEEK_SET);

        /* take care of common fields first */
        strncpy(tag->title, tagstr+3, 30); tag->title[30] = '\0';
        strncpy(tag->artist, tagstr+33, 30); tag->artist[30] = '\0';
        strncpy(tag->album, tagstr+63, 30); tag->album[30] = '\0';
        strncpy(tag->year, tagstr+93, 4); tag->year[4] = '\0';
        tag->genre = (unsigned char)tagstr[127];    /* genre is an 8-bit integer */

        if (tag->version == ID3V10) {  /* id3v1.0: 30-char comment, track number unsupported */
            tag->track = -1;
            strncpy(tag->comment, tagstr+97, 30); tag->comment[30] = '\0';
            return ID3V10;
        } else if (tag->version == ID3V11) {   /* id3v1.1: 28-char comment, track number present */
            tag->track = tagstr[126];
            strncpy(tag->comment, tagstr+97, 28); tag->comment[28] = '\0';
            return ID3V11;
        } else
            return -1;  /* in case something unexpected happens */
    }
}


void del_trail_white(char *string)
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


void sanitize_id3v1_strings(struct id3v1tag *tag)
{
    del_trail_white(tag->artist);
    del_trail_white(tag->album);
    del_trail_white(tag->title);
    del_trail_white(tag->comment);
    del_trail_white(tag->year);

    return;
}


void print_id3v1_tag(struct id3v1tag *tag)
{
    /* http://de.wikipedia.org/wiki/Liste_der_ID3v1-Genres */
    char *genres[] = {"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz",
        "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial",
        "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal",
        "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip",
        "Gospel", "Noise", "AlternRock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
        "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
        "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk",
        "Jungle", "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
        "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk",
        "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
        "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
        "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson", "Opera", "Chamber Music",
        "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
        "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo",
        "A capella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore Techno", "Terror",
        "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal",
        "Crossover", "Contemporary Christian", "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "Jpop",
        "Syntpop", "Abstract", "Art Rock", "Baroque", "Bhangra", "Big Beat", "Breakbeat", "Chillout", "Downtempo",
        "Dub", "EBM", "Electric", "Electro", "Electroclash", "Emo", "Experimental", "Garage", "Global", "IDM",
        "Illbient", "Industro-Goth", "Jam Band", "Krautrock", "Leftfield", "Lounge", "Math Rock", "New Romantic",
        "Nu-Breakz", "Post-Punk", "Post-Rock", "Psytrance", "Shoegaze", "Space Rock", "Trop Rock", "World Music",
        "Neoclassical", "Audiobook", "Audio Theatre", "Neue Deutsche Welle", "Podcast", "Indie Rock", "G-Funk",
        "Dubstep", "Garage Rock", "Psybient"};
    int n_genres = sizeof(genres)/sizeof(genres[0]);

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
    if (tag->genre >= 0 && tag->genre < n_genres)
        printf("GENRE\t%s\n", genres[tag->genre]);
    if (strlen(tag->comment) > 0)
        printf("COMMENT\t%s\n", tag->comment);

    return;
}
