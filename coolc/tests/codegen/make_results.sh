#!/bin/bash

CURR_DIR=$(pwd)
TEST_DIR=$(dirname $0) 

cd $TEST_DIR

rm -rf results
mkdir results
cd tests/
rm *$3

for file in *.cl; do
    $1/coolc $file
    filename=${file%.*}
    $2 $TEST_DIR/tests/$filename $3 $TEST_DIR/results/$file.result $1
done;

cd $CURR_DIR