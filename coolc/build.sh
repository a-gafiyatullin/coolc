#!/bin/bash

result_args=""
config=""

mode=0
test=0

c_compiler="/usr/bin/clang-10"
cxx_compiler="/usr/bin/clang++-10"

for var in "$@"
do
    case $var in
    -clean)
        echo "Clean build."
        rm -rf build/
        rm lib/*.a
        mkdir build/
        shift
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
    -test)
        test=1
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
    result_args+=" -DCMAKE_BUILD_TYPE:STRING=Debug"
    config="Debug"
fi

/usr/bin/cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE $result_args -DCMAKE_C_COMPILER:FILEPATH=$c_compiler -DCMAKE_CXX_COMPILER:FILEPATH=$cxx_compiler -H. -Bbuild -G "Unix Makefiles"
/usr/bin/cmake --build build --config $config --target all -j 10 --

if [ $test -eq 1 ]; then
    /usr/bin/make -C build test
fi