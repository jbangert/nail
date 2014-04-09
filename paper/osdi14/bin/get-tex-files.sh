#!/bin/sh

FLIST="$1"

while :;
do
    LAST="$FLIST"
    for F in $LAST; do
	NL=`cat $F | grep \\input{ | grep -v ^% | cut -d{ -f2 | cut -d} -f1`
	for F2 in $NL; do
            if [ -e "$F2" ]; then
	        FLIST="$FLIST $F2"
            else
	        FLIST="$FLIST $F2.tex"
            fi
	done
    done
    FLIST=`ls $FLIST 2>/dev/null | sort | uniq`

    if [ "$LAST" = "$FLIST" ]; then
	echo $LAST
	exit 0
    fi
done
