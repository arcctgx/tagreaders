#ifdef __CYGWIN__
#define _GNU_SOURCE     /* strdup() */
#endif

#include <string.h>
#include <string>
#include <fileref.h>
#include <tpropertymap.h>
#include "albumartist_c.h"

static char *stringToCharArray(const TagLib::String &s)
{
    const std::string str = s.to8Bit(true);
    return strdup(str.c_str());
}

char *get_album_artist(const char *path)
{
    TagLib::FileRef ref(path);
    TagLib::PropertyMap tags = ref.file()->properties();

    if (tags["ALBUMARTIST"].isEmpty() ) {
        return stringToCharArray(TagLib::String());
    }

    return stringToCharArray(tags["ALBUMARTIST"].toString());
}
