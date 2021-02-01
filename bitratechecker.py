#!/usr/bin/env python3

import argparse
import glob
import json
import os
import subprocess
import sys

class Mp3DownloadAnalyzer:
    def __init__(self, mp3_files):
        self.mp3_files = mp3_files

        if not len(self.mp3_files) > 0:
            raise ValueError

        self.name = []
        self.bitrate = []
        self.mode = []
        self.library = []
        self.encoder = []

        self.get_media_info()
        self.guess_encoder()

        self.same_bitrate = len(set(self.bitrate)) == 1
        self.same_library = len(set(self.library)) == 1
        self.same_encoder = len(set(self.encoder)) == 1
        self.all_cbr = self.mode.count("CBR") == len(self.mode)
        self.all_vbr = self.mode.count("VBR") == len(self.mode)


    def get_media_info(self):
        try:
            raw_json = subprocess.check_output(["mediainfo", "--output=JSON"] + self.mp3_files)
        except subprocess.CalledProcessError:
            raise EnvironmentError

        media = json.loads(raw_json)

        for m in media:
            self.name.append(os.path.basename(m["media"]["@ref"]))

            try:
                self.bitrate.append(int(m["media"]["track"][0]["OverallBitRate"]))
            except KeyError:
                self.bitrate.append(-1)

            try:
                self.mode.append(m["media"]["track"][0]["OverallBitRate_Mode"])
            except KeyError:
                self.mode.append("unknown")

            try:
                self.library.append(m["media"]["track"][0]["Encoded_Library"].strip())
            except KeyError:
                self.library.append("unknown")
            except AttributeError:
                self.library.append("unreadable")


    def guess_encoder(self):
        for f in self.mp3_files:
            try:
                subprocess.check_output(["mp3guessenc", "-n", f], text=True)
            except subprocess.CalledProcessError as e:
                # mp3guessenc returns different statuses for different encoders - not just zero on success
                # so we actually only get meaningful output here in except block
                enc = e.output.splitlines()[0]

            self.encoder.append(enc)


    def __str__(self):
        s = []

        for z in zip(self.name, self.bitrate, self.mode, self.library, self.encoder):
            s.append("{:<50}\t{}\t{:<12}\t{:<20}\t{}".format(*z))
        s.append("")

        return "\n".join(s)


    def is_uniform(self):
        if not self.same_encoder:
            return False

        if not self.same_library:
            return False

        if not self.all_cbr and not self.all_vbr:
            return False

        if self.all_cbr and not self.same_bitrate:
            return False

        return True


def parse_cmdline_args():
    parser = argparse.ArgumentParser(
            prog="bitratechecker",
            description="""Check uniformity of a set of .mp3 files inside a directory.
                Uniformity is verified based on bitrate mode (all VBR or all CBR), bitrate
                values (all CBR files must have the same bitrate), encoder strings reported
                by mediainfo (must be the same for each file), and encoder type reported
                by mp3guessenc (must be the same as well). A summary is printed if the set
                is not uniform.""")
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
            "-q", "--quiet", action="store_true",
            help="never print the summary")
    group.add_argument(
            "-v", "--verbose", action="store_true",
            help="always print the summary")
    parser.add_argument(
            "directory", default=[os.getcwd()], nargs="*",
            help="""path to a directory with .mp3 files to verify
                (default: current directory)""")

    return parser.parse_args()


def main():
    args = parse_cmdline_args()

    for path in args.directory:
        if not os.path.isdir(path):
            continue

        mp3_files = sorted(glob.glob(os.path.join(path, "*.[mM][pP]3")))

        print("Checking", path, "-", end=' ')

        try:
            download = Mp3DownloadAnalyzer(mp3_files)
            if not download.is_uniform():
                print("NOK")
                if not args.quiet:
                    print(download)
            else:
                print("OK")
                if args.verbose:
                    print(download)
        except ValueError:
            print("no .mp3 files found!")
        except EnvironmentError:
            print("failed to get metadata using mediainfo!")


if __name__ == "__main__":
    main()
