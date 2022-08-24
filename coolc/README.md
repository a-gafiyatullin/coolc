# The COOL Programming Language Compiler

This directory and its subdirectories contain source code for **coolc**,
the compiler for **Classroom Object Oriented Language** created for educational purposes by **Alexander Aiken**.

Please see the documentation provided in **docs/** for further
assistance with **coolc**.

## Getting the Source Code and Building COOLC

1. Checkout COOLC:
    - `git clone https://github.com/xp10rd/Compilers.git`

2. Configure and build COOLC:
    - Prerequirement: **clang**, **gtest**, **llvm**;
    - `cd coolc`
    - `build.sh [-release/-debug] [options]`<br>
    Some usefull options:
        - `-clean` --- clean build directory before compile.
        - `-asan` --- build with AddressSanitizer (Debug only).
        - `-ubsan` --- build with UndefinedBehaviorSanitizer (Debug only).
        - `-test` --- run tests after building.
        - `-mips` --- build for SPIM emulator.
        - `-llvm` --- build with **LLVM** for host architecture.

3. Some usefull runtime options for **LLVM**-based build (pass them as argument to executable):
   1. `MaxHeapSize` --- maximal heap size (e.g. `MaxHeapSize=1024[Gb/Mb/Kb/no specifier for bytes]`).
   2. `GCAlgo` --- GC algorithm (e.g. `GCAlgo=1`):
      1. `ZeroGC` (code **0**) --- just allocate memory without collecting.
      2. `MarkSweepGC` (code **1**) --- use Mark-and-Sweep Garbage Collector.
         1. For now all collectors use **LLVM Shadow Stack**.
         2. Default collector is `MarkSweepGC`.
   3. `PrintGCStatistics` --- (**debug only**) print amount of allocated and freed memory (e.g. `+PrintGCStatistics`).
   4. `PrintAllocatedObjects` --- (**debug only**) print object layout on every allocation (e.g. `+PrintAllocatedObjects`).