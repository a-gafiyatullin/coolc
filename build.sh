#!/bin/bash

result_args=""
config=""

mode=0
test=0

c_compiler="/usr/bin/clang"
cxx_compiler="/usr/bin/clang++"

for var in "$@"
do
    case $var in
    -clean)
        echo "Clean build."
        rm -rf build/
        rm lib/*.a
        mkdir build/
        exit 0
    ;;
    -release)
        mode=1
        result_args+=" -DCMAKE_BUILD_TYPE:STRING=Release"
        config="Release"
        shift
    ;;
    -debug)
        mode=1
        result_args+=" -DCMAKE_BUILD_TYPE:STRING=Debug"
        config="Debug"
        shift
    ;;
    -asan)
        result_args+=" -DASAN=ON"
        shift
    ;;
    -ubsan)
        result_args+=" -DUBSAN=ON"
        shift
    ;;
    -mips)
        result_args+=" -DARCH:STRING=MIPS"
        shift
    ;;
    -no-gc)
        result_args+=" -DGCTYPE:STRING=NO_GC"
        shift
    ;;
    -shadow-stack-gc)
        result_args+=" -DGCTYPE:STRING=LLVM_SHADOW_STACK"
        shift
    ;;
    -statepoint-example-gc)
        result_args+=" -DGCTYPE:STRING=LLVM_STATEPOINT_EXAMPLE"
        shift
    ;;
    -test)
        test=1
        shift
    ;;
    -llvm)
        result_args+=" -DARCH:STRING=LLVM"
        shift
    ;;
     -myir)
        result_args+=" -DARCH:STRING=MYIR"
        shift
    ;;
    -*|--*)
        echo "Unknown option $var"
        exit 1
    ;;
    *)
    ;;
    esac
done

if [ $mode -eq 0 ]; then
    result_args+=" -DCMAKE_BUILD_TYPE:STRING=Release"
    config="Release"
fi

cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE $result_args -DCMAKE_C_COMPILER:FILEPATH=$c_compiler -DCMAKE_CXX_COMPILER:FILEPATH=$cxx_compiler -H. -Bbuild -G "Unix Makefiles"
cmake --build build --config $config --target all -j 10 --

ln -sf build/compile_commands.json compile_commands.json

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/$PWD/bin/
    echo "Added runtime lib to your LD_LIBRARY_PATH = " $LD_LIBRARY_PATH
elif [[ "$OSTYPE" == "darwin"* ]]; then
    export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH:/$PWD/bin/
    echo "Added runtime lib to your DYLD_LIBRARY_PATH = " $DYLD_LIBRARY_PATH
fi

if [ $test -eq 1 ]; then
    ctest --test-dir build -C $config
fi
