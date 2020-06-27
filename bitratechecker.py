#!/usr/bin/env python2

from __future__ import print_function
import sys
import os
import glob
import subprocess
import json
import argparse

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
        print()


    def is_uniform(self):
        if not self.__same_encoder:
            return False

        if not self.__same_library:
            return False

        if not self.__all_cbr and not self.__all_vbr:
            return False

        if self.__all_cbr and not self.__same_bitrate:
            return False

        return True


parser = argparse.ArgumentParser(
        prog="bitratechecker",
        description="""Check uniformity of a set of .mp3 files inside a directory.
            Uniformity is verified based on bitrate mode (all VBR or all CBR), bitrate
            values (all CBR files must have the same bitrate), encoder strings reported
            by mediainfo (must be the same for each file), and encoder type reported
            by mp3guessenc (must be the same as well). A summary is printed if the set
            is not uniform.""")
parser.add_argument(
        "-v", "--verbose", action="store_true",
        help="print the summary also for uniform sets")
parser.add_argument(
        "directory", default=[os.getcwd()], nargs="*",
        help="""path to a directory with .mp3 files to verify
            (default: current directory)""")
args = parser.parse_args()

for path in args.directory:
    if not os.path.isdir(path):
        continue

    mp3_files = sorted(glob.glob(os.path.join(path, "*.[mM][pP]3")))

    print("Checking", path, "-", end=' ')

    try:
        download = Mp3DownloadAnalyzer(mp3_files)
        if not download.is_uniform():
            print("NOK")
            download.print_formatted()
        else:
            print("OK")
            if args.verbose:
                download.print_formatted()
    except ValueError:
        print("no .mp3 files found!")
    except EnvironmentError:
        print("failed to get metadata using mediainfo!")
