#!/bin/bash
make && ./nail test.nail parser_test.c -parser_hammer && cat testharness.c >> parser_test.c && astyle parser_test.c || exit 
for line in $(ls tests|sort -n)
do
    IFS='.' read -ra NAME <<<"$line"
    var="${NAME[1]}"
    case "$var" in 
        [A-Z]* ) const="1";;
        [a-z]* ) const="0";;
        *) echo "Can't determine constness of $var" ; exit 1;;
    esac
    gcc -ggdb -std=gnu99 -DXYCONST=$const -DXYZZY=$var -o testbin/$var parser_test.c  || exit -1
    echo "RUNNING TEST: " $line
    testbin/$var < tests/$line || exit -1
    echo "SUCCESS"
done
