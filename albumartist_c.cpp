#ifdef __CYGWIN__
#define _GNU_SOURCE     /* strdup() */
#endif

#include <string.h>
#include <string>
#include <tag.h>
#include <fileref.h>
#include <tpropertymap.h>
#include <tag_c.h>
#include "albumartist_c.h"

static char *stringToCharArray(const TagLib::String &s)
{
    const std::string str = s.to8Bit(true);
    return strdup(str.c_str());
}

char *get_album_artist(const TagLib_File *file)
{
    const TagLib::File *f = reinterpret_cast<const TagLib::File*>(file);

    TagLib::PropertyMap tags = f->properties();

    if (tags["ALBUMARTIST"].isEmpty() ) {
        return stringToCharArray(TagLib::String());
    }

    return stringToCharArray(tags["ALBUMARTIST"].toString());
}
