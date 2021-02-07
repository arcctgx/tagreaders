#!/usr/bin/env python3

import re
import subprocess
from datetime import datetime

STAT_PATH = '/home/arcctgx/Dropbox/data/beet_stats.txt'

output = subprocess.check_output(['beet', 'stats', '-e'], text=True).splitlines()

tracks= int(output[0].split(':')[1])
artists = int(output[3].split(':')[1])
albums = int(output[4].split(':')[1])
album_artists = int(output[5].split(':')[1])

# a sequence of any characters between opening ( and first space:
regex = re.compile(r'\((.+)\ ')

seconds = float(regex.findall(output[1])[0])
size = int(regex.findall(output[2])[0])

current_time = datetime.now()
iso_time = current_time.isoformat()
unix_time = current_time.strftime('%s')

with open(STAT_PATH, 'a') as f:
    f.write('%s %s %d %d %d %d %.2f %d\n' %
        (unix_time, iso_time, artists, album_artists, albums, tracks, seconds, size))
