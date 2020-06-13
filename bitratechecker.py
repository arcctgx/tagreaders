#!/usr/bin/env python2

import sys
import os
import glob
import subprocess
import json

def usage():
    print "usage: " + os.path.basename(sys.argv[0]) + " <dir> [dir2] ..."
    sys.exit(1)

def get_encoder(file_list):
    encoder = []

    for f in sorted(file_list):
	# mp3guessenc returns different statuses for different encoders...
        try:
            enc = subprocess.check_output(["mp3guessenc", "-n", f]).splitlines()[0]
        except subprocess.CalledProcessError, e:
            enc = e.output.splitlines()[0]

        encoder.append(enc)

    return encoder


if len(sys.argv) < 2:
    usage()

for path in sys.argv[1:]:
    print path

    mp3_files = glob.glob(os.path.join(path, "*.mp3"))

    raw_json = subprocess.check_output(["mediainfo", "--output=JSON"] + sorted(mp3_files))
    media = json.loads(raw_json)

    name = []
    bitrate = []
    mode = []
    library = []

    for m in media:
        name.append(os.path.basename(str(m["media"]["@ref"])))

        try:
            bitrate.append(int(m["media"]["track"][0]["OverallBitRate"]))
        except KeyError:
            bitrate.append(-1)

        try:
            mode.append(str(m["media"]["track"][0]["OverallBitRate_Mode"]))
        except KeyError:
            mode.append("unknown")

        try:
            library.append(str(m["media"]["track"][0]["Encoded_Library"]))
        except KeyError:
            library.append("unknown")

    print name
    print bitrate
    print mode
    print library

    encoder = get_encoder(mp3_files)
    print encoder

    print
