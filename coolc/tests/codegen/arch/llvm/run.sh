#!/bin/bash

clang++ $2/$5.o $1/libcool-rt.so -o $4/$5

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    LD_LIBRARY_PATH=$1 $4/$5 &> $3
elif [[ "$OSTYPE" == "darwin"* ]]; then
    DYLD_LIBRARY_PATH=$1 $4/$5 &> $3
fi