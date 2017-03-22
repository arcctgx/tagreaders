#!/usr/bin/env python

import sys
import os
import mutagen

def usage():
    print "usage: " + os.path.basename(sys.argv[0]) + " <mp3file>"
    sys.exit(11)

if len(sys.argv) != 2:
    usage()

myfile = sys.argv[1]

try:
    audio = mutagen.File(myfile)
except Exception, e:
    print "ERR: %s" % (e)
    sys.exit(12)

try:
    audio.info
except AttributeError:
    print "ERR: \"%s\" is not an audio file!" % (myfile)
    sys.exit(13)

if type(audio.info) == mutagen.mp3.MPEGInfo:
    if hasattr(audio.info, "bitrate_mode"):
        vbr = audio.info.bitrate_mode == mutagen.mp3.BitrateMode.VBR
    else:
        if audio.info.bitrate % 1000 != 0:
            vbr = True
else:
    print "ERR: %s is not mp3 file!" % (myfile)
    sys.exit(14)

if vbr == True:
    print "INF: %s is VBR :)" % myfile
    sys.exit(0)
else:
    print "INF: %s is NOT VBR" % myfile
    sys.exit(1)
