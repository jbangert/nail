#!/bin/bash
pushd ../generator 
make 
popd
for i in */*.nail
do 
    ./harness.sh $i
done

