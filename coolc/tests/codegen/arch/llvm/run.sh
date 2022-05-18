#!/bin/bash

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    LD_LIBRARY_PATH=$1 $4/$5 &> $3
elif [[ "$OSTYPE" == "darwin"* ]]; then
    DYLD_LIBRARY_PATH=$1 $4/$5 &> $3
fi