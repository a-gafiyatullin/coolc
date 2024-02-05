#!/bin/bash

CURR_DIR=$(pwd)
TEST_DIR=$(dirname $0) 

cd $TEST_DIR 

rm -rf results
mkdir results
rm -rf out
mkdir out
cd tests/

for file in *.test; do
    $1/coolc +PrintFinalAST $file -o $TEST_DIR/out/$file.out &> $TEST_DIR/results/$file.result
done;

cd $CURR_DIR