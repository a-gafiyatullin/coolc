#!/bin/bash

clang++ $1$2 $4/libcool-rt.so -o $1

LD_LIBRARY_PATH=$4 $1 &> $3