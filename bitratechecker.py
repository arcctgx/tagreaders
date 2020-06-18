#!/usr/bin/env python2

from __future__ import print_function
import sys
import os
import glob
import subprocess
import json

class Mp3DownloadAnalyzer(object):
    def __init__(self, mp3_files):

        if not len(mp3_files) > 0:
            raise ValueError

        self.__name = []
        self.__bitrate = []
        self.__mode = []
        self.__library = []
        self.__encoder = []

        self.__get_media_info(mp3_files)
        self.__guess_encoder(mp3_files)

        self.__same_bitrate = len(set(self.__bitrate)) == 1
        self.__same_library = len(set(self.__library)) == 1
        self.__same_encoder = len(set(self.__encoder)) == 1
        self.__all_cbr = self.__mode.count("CBR") == len(self.__mode)
        self.__all_vbr = self.__mode.count("VBR") == len(self.__mode)

        #print("DBG:", self.__same_bitrate, self.__same_library, self.__same_encoder, self.__all_cbr, self.__all_vbr)


    def __get_media_info(self, mp3_files):
        try:
            raw_json = subprocess.check_output(["mediainfo", "--output=JSON"] + mp3_files)
        except subprocess.CalledProcessError:
            raise EnvironmentError

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
                self.__library.append(str(m["media"]["track"][0]["Encoded_Library"].encode("utf-8").strip()))
            except KeyError:
                self.__library.append("unknown")
            except AttributeError:
                self.__library.append("unreadable")


    def __guess_encoder(self, mp3_files):
        for f in mp3_files:
            try:
                enc = subprocess.check_output(["mp3guessenc", "-n", f]).splitlines()[0]
            except subprocess.CalledProcessError as e:
                # mp3guessenc returns different statuses for different encoders - not just zero on success
                # so we actually get meaningful output here in except block
                enc = e.output.splitlines()[0]

            self.__encoder.append(enc)


    def print_formatted(self):
        s = []

        for z in zip(self.__name, self.__bitrate, self.__mode, self.__library, self.__encoder):
            s.append("{:<50}\t{}\t{:<12}\t{:<20}\t{}".format(z[0], z[1], z[2], z[3], z[4]))

        print('\n'.join(s))


    def is_uniform(self):
        if not self.__same_encoder:
            print("different encoders")
            return False

        if not self.__same_library:
            print("different libraries")
            return False

        if not self.__all_cbr and not self.__all_vbr:
            print("mixed VBR and CBR")
            return False

        if self.__all_cbr and not self.__same_bitrate:
            print("all CBR but different bitrates")
            return False

        return True


def usage():
    print("usage:", os.path.basename(sys.argv[0]), "<dir> [dir2] ...")
    sys.exit(1)


if len(sys.argv) < 2:
    usage()

for path in sys.argv[1:]:
    mp3_files = sorted(glob.glob(os.path.join(path, "*.mp3")))

    try:
        download = Mp3DownloadAnalyzer(mp3_files)
        if not download.is_uniform():
            print(path)
            download.print_formatted()
            print()
    except ValueError:
        print("directory", path, "does not contain MP3 files!")
    except EnvironmentError:
        print("failed to get metadata of files in", path, "using mediainfo!")
