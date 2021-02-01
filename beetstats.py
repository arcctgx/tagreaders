#!/usr/bin/env python

from datetime import datetime
import subprocess
import re

stat_path = "/home/arcctgx/Dropbox/data/beet_stats.txt"

output = subprocess.check_output(["beet", "stats", "-e"]).splitlines()

tracks= int(output[0].split(":")[1])
artists = int(output[3].split(":")[1])
albums = int(output[4].split(":")[1])
album_artists = int(output[5].split(":")[1])

# a sequence of any characters between opening ( and first space:
regex = re.compile("\((.+)\ ")

seconds = float(regex.findall(output[1])[0])
size = int(regex.findall(output[2])[0])

hours = seconds/3600
size_MiB = float(size)/1024/1024

current_time = datetime.now()
unix_time = current_time.strftime("%s")

statfile = open(stat_path, "a")
print >> statfile, "%s %s %d %.2f %d %d %d %d %.2f %.2f" % (current_time.isoformat(), unix_time, tracks, seconds, size, artists, albums, album_artists, hours, size_MiB)
statfile.close()
