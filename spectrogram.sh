#!/bin/bash

make_spectrogram() {
    local -r xsize=1750
    local -r comment="$(basename "${1}")"
    local -r outputFile="$(basename "${1}").png"

    sox "${1}" -n spectrogram -l -x "${xsize}" -c "${comment}" -o "${outputFile}"
}

if [ "${#}" -eq 0 ]; then
    echo "usage: $(basename $0) <file> [file2 ...]"
    exit
fi

while [ "${#}" -gt 0 ]
do
    echo "${1}"
    make_spectrogram "${1}"
    shift
done
