#!/bin/sh
cd $(dirname $1)
TEST=$(basename $1)

echo "Running $TEST"
../../generator/nail $TEST   2&>/dev/null || exit 
cat ../test_harness.c >> $TEST.c
astyle $TEST.c
rm $TEST.c.orig
gcc  -ggdb -DXYCONST=0 -lhammer -std=gnu99 -o $TEST-test $TEST.c 
    
for case in in.*
 do 
   ./$TEST-test < $case
 done

