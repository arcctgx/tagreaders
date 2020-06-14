#!/usr/bin/env python2

import sys
import os
import glob
import subprocess
import json

class bitratechecker(object):
    def __init__(self, file_list):
        self.__name = []
        self.__bitrate = []
        self.__mode = []
        self.__library = []
        self.__encoder = []

        if len(file_list) < 1:
            raise ValueError

        try:
            raw_json = subprocess.check_output(["mediainfo", "--output=JSON"] + sorted(file_list))
        except subprocess.CalledProcessError:
            raise IOError

        media = json.loads(raw_json)

        for m in media:
            self.__name.append(os.path.basename(str(m["media"]["@ref"])))

            try:
                self.__bitrate.append(int(m["media"]["track"][0]["OverallBitRate"]))
            except KeyError:
                self.__bitrate.append(-1)

            try:
                self.__mode.append(str(m["media"]["track"][0]["OverallBitRate_Mode"]))
            except KeyError:
                self.__mode.append("unknown")

            try:
                self.__library.append(str(m["media"]["track"][0]["Encoded_Library"].encode("utf-8")))
            except KeyError:
                self.__library.append("unknown")
            except AttributeError:
                self.__library.append("unreadable")

        for f in sorted(file_list):
            try:
                enc = subprocess.check_output(["mp3guessenc", "-n", f]).splitlines()[0]
            except subprocess.CalledProcessError, e:
                # mp3guessenc returns different statuses for different encoders - not just zero on success
                # so we actually get meaningful output here in except block
                enc = e.output.splitlines()[0]

            self.__encoder.append(enc)

    def print_formatted(self):
        s = []

        for z in zip(self.__name, self.__bitrate, self.__mode, self.__library, self.__encoder):
            s.append("%-50s\t%d\t%-12s\t%-20s\t%s" % (z[0], z[1], z[2], z[3], z[4]) )

        print '\n'.join(s)

def usage():
    print "usage: " + os.path.basename(sys.argv[0]) + " <dir> [dir2] ..."
    sys.exit(1)


if len(sys.argv) < 2:
    usage()

for path in sys.argv[1:]:
    print path

    mp3_files = glob.glob(os.path.join(path, "*.mp3"))

    try:
        x = bitratechecker(mp3_files)
        x.print_formatted()
    except ValueError:
        print "directory " + path + " does not contain MP3 files!"
    except IOError:
        print "failed to get metadata of files in " + path + " using mediainfo!"

    print
