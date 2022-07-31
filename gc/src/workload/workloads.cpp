#include "workloads.hpp"
#include "gc/gc-interface/alloca.hpp"
#include "gc/gc-interface/gc.hpp"

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
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC<allocator::Alloca>, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>, VERY_SMALL_HEAP>));

    RUN_TEST(
        (sanity_trivial_collect_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, VERY_SMALL_HEAP>));
    RUN_TEST(
        (sanity_trivial_collect_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>,
                                              VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // INTEL: GC_MARK: 1, GC_SWEEP: 0, ALLOCATION: 3, EXECUTION: 1
    // RPI4:  GC_MARK: 5, GC_SWEEP: 2, ALLOCATION: 5, EXECUTION: 12
    // MACM1: GC_MARK: 1, GC_SWEEP: 1, ALLOCATION: 1, EXECUTION: 3
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 0, GC_SWEEP: 0, ALLOCATION: 1, EXECUTION: 3
    // RPI4:  GC_MARK: 3, GC_SWEEP: 1, ALLOCATION: 11, EXECUTION: 6
    // MACM1: GC_MARK: 1, GC_SWEEP: 0, ALLOCATION: 1, EXECUTION: 4
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 0, GC_SWEEP: 1, ALLOCATION: 1, EXECUTION: 4
    // RPI4:  GC_MARK: 1, GC_SWEEP: 3, ALLOCATION: 9, EXECUTION: 7
    // MACM1: GC_MARK: 2, GC_SWEEP: 0, ALLOCATION: 0, EXECUTION: 3
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // ---

    // INTEL: GC_MARK: 325, GC_SWEEP: 161, ALLOCATION: 979, EXECUTION: 957
    // RPI4:  GC_MARK: 2914, GC_SWEEP: 1206, ALLOCATION: 4360, EXECUTION: 4482
    // MACM1: GC_MARK: 407, GC_SWEEP: 126, ALLOCATION: 624, EXECUTION: 1105
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, MEDIUM_HEAP,
                                              MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 226, GC_SWEEP: 161, ALLOCATION: 1015, EXECUTION: 960
    // RPI4:  GC_MARK: 2076, GC_SWEEP: 1220, ALLOCATION: 4299, EXECUTION: 4402
    // MACM1: GC_MARK: 402, GC_SWEEP: 124, ALLOCATION: 646, EXECUTION: 1077
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, MEDIUM_HEAP,
                                              MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 249, GC_SWEEP: 160, ALLOCATION: 1030, EXECUTION: 949
    // RPI4:  GC_MARK: 2186, GC_SWEEP: 1229, ALLOCATION: 4309, EXECUTION: 4370
    // MACM1: GC_MARK: 416, GC_SWEEP: 126, ALLOCATION: 650, EXECUTION: 1068
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>,
                                              MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // ---

    // INTEL: GC_MARK: 651, GC_SWEEP: 324, ALLOCATION: 1975, EXECUTION: 1919
    // RPI4:  GC_MARK: 5641, GC_SWEEP: 2443, ALLOCATION: 8745, EXECUTION: 8862
    // MACM1: GC_MARK: 847, GC_SWEEP: 250, ALLOCATION: 1329, EXECUTION: 2130
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 469, GC_SWEEP: 330, ALLOCATION: 2122, EXECUTION: 1917
    // RPI4:  GC_MARK: 4010, GC_SWEEP: 2430, ALLOCATION: 8575, EXECUTION: 8974
    // MACM1: GC_MARK: 789, GC_SWEEP: 251, ALLOCATION: 1256, EXECUTION: 2183
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 496, GC_SWEEP: 332, ALLOCATION: 2137, EXECUTION: 1911
    // RPI4:  GC_MARK: 4044, GC_SWEEP: 2443, ALLOCATION: 8626, EXECUTION: 8851
    // MACM1: GC_MARK: 836, GC_SWEEP: 252, ALLOCATION: 1297, EXECUTION: 2139
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));
}