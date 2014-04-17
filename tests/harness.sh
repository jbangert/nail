#!/bin/sh 
cd $(dirname $1)
TEST=$(basename $1)

echo "Running $TEST"
NAIL=`../../generator/nail $TEST   2>&1`  || ( echo "Failed generating $case \n"  $NAIL; exit -1) || echo $NAIL
CFILE="${TEST%.*}"
if [ -e $CFILE.c ]; then
    echo "Adding transform functions from $CFILE"
    cat  $CFILE.c >> $TEST.c
fi
cat ../test_harness.c >> $TEST.c
astyle $TEST.c > /dev/null

rm $TEST.c.orig
COMPILER=`gcc  -ggdb -DXYCONST=0 -lhammer -std=gnu99 -o $TEST-test $TEST.c  2>&1` || (echo "Failed compiling $case \n $COMPILER"; exit -1 ) || exit -1 
    
for case in $CFILE.in.*
 do 
   OUT=`./$TEST-test < $case 2>&1` || ( echo "Failed  testcase $case \n" $OUT; exit -1) || exit -1 
 done

