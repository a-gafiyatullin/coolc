#include "workloads.hpp"

// Machines:
//
// INTEL:
// WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
// clang version 10.0.0-4ubuntu1
//
// RPI4:
// rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
// clang version 10.0.0-4ubuntu1
//
// MACM1:
// Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4, Power supply
// Apple clang version 13.1.6 (clang-1316.0.21.2.5)

void sanity_workload()
{
    sanity_trivial_workload<gc::ZeroGC<allocator::Alloca, IIS_HEADER>, IIS_HEADER>(VERY_SMALL_HEAP);

    sanity_trivial_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO, IIS_HEADER>, IIS_HEADER>(
        VERY_SMALL_HEAP);

    sanity_trivial_collect_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO, IIS_HEADER>, IIS_HEADER>(
        VERY_SMALL_HEAP);

    sanity_trivial_workload<gc::Lisp2GC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO, AIS_HEADER>, AIS_HEADER>(
        VERY_SMALL_HEAP);
}

void linked_list_workload()
{
    // INTEL: GC_MARK: 456, GC_MAIN_PHASE: 312, ALLOCATION: 1800, EXECUTION: 1915
    // RPI4:  GC_MARK: 3902, GC_MAIN_PHASE: 2389, ALLOCATION: 7810, EXECUTION: 8221
    // MACM1: GC_MARK: 824, GC_MAIN_PHASE: 244, ALLOCATION: 1220, EXECUTION: 3531
    RUN_TEST1((linked_list_allocation_workload<gc::MarkSweepGC, allocator::NextFitAlloca, gc::MarkerFIFO, IIS_HEADER,
                                               false, 0>),
              BIG_HEAP, BIG_LINKED_LIST);

    // INTEL: GC_MARK: 297, GC_MAIN_PHASE: 1000, ALLOCATION: 1595, EXECUTION: 1750
    // RPI4:  GC_MARK: 2647, GC_MAIN_PHASE: 6536, ALLOCATION: 6473, EXECUTION: 7811
    // MACM1: GC_MARK: 480, GC_MAIN_PHASE: 712, ALLOCATION: 1041, EXECUTION: 3124
    RUN_TEST1(
        (linked_list_allocation_workload<gc::Lisp2GC, allocator::NextFitAlloca, gc::MarkerFIFO, AIS_HEADER, false, 0>),
        1.14 * BIG_HEAP, BIG_LINKED_LIST);
}