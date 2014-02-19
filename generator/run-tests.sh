#!/bin/bash
make
for line in $(ls tests|sort -rn)
do
    IFS='.' read -ra NAME <<<"$line"
    var="${NAME[1]}"
    g++ -lhammer -ggdb -std=gnu++11 -DX=$var -o testbin/$var parser_test.cc foo.o || exit -1
    echo "RUNNING TEST: " $line
    testbin/$var < tests/$line || exit -1
    echo "SUCCESS"
done
