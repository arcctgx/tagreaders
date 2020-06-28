#!/bin/bash

set -e

function usage {
    echo " usage: $(basename $0) [-n|--dry-run] <directory>"
    exit
}

function sanitize {
    if [ "$2" != "dir" ]; then
        echo "$1" | \
            tr [[:upper:]] [[:lower:]] | \
            tr [±æêé³ñó¶¿¼äöü\ ] [aceelnoszzaou_] | \
            sed 's/&/and/g' | \
            sed -r 's/(.)[-=]/\1_/g' | \
            tr -cd _a-z0-9 | \
            sed 's/__*/_/g'
    else
        echo "$1" | \
            tr [[:upper:]] [[:lower:]] | \
            tr [±æêé³ñó¶¿¼äöü\ ] [aceelnoszzaou_] | \
            sed 's/&/and/g' | \
            sed -r 's/([:graph:])[-=]/\1_/g' | \
            tr -cd _a-z0-9-
    fi
}

function fname {
    name=$(echo "$1" | rev | cut -d. -f2- | rev)
    exte=$(echo "$1" | rev | cut -d. -f1 | rev)
    echo "$(sanitize "${name}")"."$(sanitize "${exte}")"
}

function dname {
    echo "$(sanitize "$1" dir)"
}


while [ "$1" != "" ]; do
    case $1 in
        -n|--dry-run)
            dry_run="YES"
        ;;
        -*)
            usage
        ;;
        *)
            dirname=$1
        ;;
    esac
    shift
done

if [ "${dirname}" = "" ]; then
    usage
fi

if [ ! -d "${dirname}" ]; then
    echo " directory \"${dirname}\" does not exist."
    exit
fi


dirsanit="$(dname "${dirname}")"

if [ "$dirsanit" = "$dirname" ]; then
    echo "\"$dirname\" is sane"
    dirsanit="$dirname"
else
    if [ "${dry_run}" != "YES" ]; then
        mv -v "${dirname}" "${dirsanit}"
    else
        echo "${dirname} -> ${dirsanit}"
        dirsanit="${dirname}"
    fi
fi

(
    cd "${dirsanit}"

    find . -maxdepth 1 -type f | sort | cut -c3- | \
        while read filename
        do
            newname=$(fname "${filename}")
            if [ "${filename}" = "${newname}" ]; then 
                echo "\"${filename}\" is sane"
            else
                if [ "${dry_run}" = "YES" ]; then
                    echo "${filename}" -> "${newname}"
                else
                    mv -v "${filename}" "${newname}"
                fi
            fi
        done

    if [ "${dry_run}" != "YES" ]; then
        tmpm3u=$(mktemp)
    fi

    find . -maxdepth 1 -name "*.m3u" -type f | \
        while read playlist
        do
            while read line
            do
                if [ ${line:0:1} != "#" ]; then
                    if [ "${dry_run}" != "YES" ]; then
                        echo "$(fname "${line}" | sed 's/$/\r/')" >> "${tmpm3u}"
                    else
                        echo "$(fname "${line}" | sed 's/$/\r/')"
                    fi
                else
                    if [ "${dry_run}" != "YES" ]; then
                        echo "${line}" >> "${tmpm3u}"
                    else
                        echo "${line}"
                    fi
                fi
            done < "${playlist}"
            if [ "${dry_run}" != "YES" ]; then
                cp -v "${tmpm3u}" "${playlist}"
            fi
        done

    if [ "${dry_run}" != "YES" ]; then
        rm -v "${tmpm3u}"
    fi
)
