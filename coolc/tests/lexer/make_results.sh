#!/bin/bash

CURR_DIR=$(pwd)
TEST_DIR=$(dirname $0) 

cd $TEST_DIR

rm -rf results
mkdir results
cd tests/

for file in *.cool; do
    $1/coolc +TokensOnly $file &> $TEST_DIR/results/$file.result
done;

cd $CURR_DIR