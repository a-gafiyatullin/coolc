#!/bin/bash

clang++ $2/$5.o $1/libcool-rt.so -o $4/$5

LD_LIBRARY_PATH=$1 $4/$5 &> $3