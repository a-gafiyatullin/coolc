#!/bin/bash

CURR_DIR=$(pwd)
TEST_DIR=$(dirname $0) 

cd $TEST_DIR

rm -rf results
rm -rf out
mkdir results
mkdir out

cd tests/

for file in *.cl; do
    $1/coolc $file
    filename=${file%.*}
    $2 $1 $TEST_DIR/tests/ $TEST_DIR/results/$file.result $TEST_DIR/out/ $filename
done;

cd $CURR_DIR