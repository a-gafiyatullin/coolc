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

DECLARE_TEST(sanity)
{
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC<allocator::Alloca, IIS_HEADER>, IIS_HEADER, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO, IIS_HEADER>, IIS_HEADER,
                                      VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO, IIS_HEADER>,
                                              IIS_HEADER, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_workload<gc::Lisp2GC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO, AIS_HEADER>, AIS_HEADER,
                                      VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // INTEL: GC_MARK: 458, GC_MAIN_PHASE: 311, ALLOCATION: 1900, EXECUTION: 1858
    // RPI4:  GC_MARK: 4010, GC_SWEEP: 2430, ALLOCATION: 8575, EXECUTION: 8974
    // MACM1: GC_MARK: 789, GC_SWEEP: 251, ALLOCATION: 1256, EXECUTION: 2183
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO, IIS_HEADER>,
                                              IIS_HEADER, BIG_HEAP, BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 406, GC_MAIN_PHASE: 1312, ALLOCATION: 1500, EXECUTION: 1829
    RUN_TEST(
        (linked_list_allocation_workload<gc::Lisp2GC<allocator::NextFitAlloca, gc::MarkerFIFO, AIS_HEADER>, AIS_HEADER,
                                         BIG_HEAP + (sizeof(AIS_HEADER) - sizeof(IIS_HEADER)) * BIG_LINKED_LIST,
                                         BIG_LINKED_LIST, false, 0>));
}