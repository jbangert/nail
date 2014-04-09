#!/bin/sh

if [ "$1" = "" ]; then
    echo "usage: `basename $0` <file> ..."
    exit
fi

grep --color ly- "$@"
exit 0

