FROM --platform=linux/amd64 ubuntu:22.04

RUN apt update && apt upgrade
RUN apt install -y llvm clang clang-tidy cmake libboost-dev \
                   libboost-filesystem-dev libgtest-dev nano 

WORKDIR /gtest_build
RUN cmake -DCMAKE_BUILD_TYPE=Release /usr/src/gtest && cmake --build . && mv lib/*.a /usr/lib

RUN useradd -ms /bin/bash xp10rd
USER xp10rd
WORKDIR /home/xp10rd/coolc