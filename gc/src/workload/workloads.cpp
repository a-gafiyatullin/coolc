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
    // INTEL: GC_MARK: 0, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    // RPI4:  GC_MARK: 6, GC_SWEEP: 1, ALLOCATION: 8, EXECUTION: 10
    // MACM1: GC_MARK: 0, GC_SWEEP: 3, ALLOCATION: 3, EXECUTION: 7
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 1, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    // RPI4:  GC_MARK: 3, GC_SWEEP: 2, ALLOCATION: 11, EXECUTION: 6
    // MACM1: GC_MARK: 0, GC_SWEEP: 1, ALLOCATION: 6, EXECUTION: 4
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 0, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    // RPI4:  GC_MARK: 3, GC_SWEEP: 2, ALLOCATION: 9, EXECUTION: 7
    // MACM1: GC_MARK: 1, GC_SWEEP: 1, ALLOCATION: 3, EXECUTION: 4
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>, SMALL_HEAP,
                                              SMALL_LINKED_LIST, false, 0>));

    // ---

    // INTEL: GC_MARK: 343, GC_SWEEP: 166, ALLOCATION: 1130, EXECUTION: 976
    // RPI4:  GC_MARK: 2935, GC_SWEEP: 1218, ALLOCATION: 4978, EXECUTION: 4118
    // MACM1: GC_MARK: 396, GC_SWEEP: 122, ALLOCATION: 740, EXECUTION: 1111
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, MEDIUM_HEAP,
                                              MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 236, GC_SWEEP: 163, ALLOCATION: 1160, EXECUTION: 1008
    // RPI4:  GC_MARK: 2214, GC_SWEEP: 1547, ALLOCATION: 5029, EXECUTION: 4110
    // MACM1: GC_MARK: 405, GC_SWEEP: 123, ALLOCATION: 739, EXECUTION: 1095
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, MEDIUM_HEAP,
                                              MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 248, GC_SWEEP: 159, ALLOCATION: 1188, EXECUTION: 1010
    // RPI4:  GC_MARK: 2544, GC_SWEEP: 1799, ALLOCATION: 4960, EXECUTION: 4107
    // MACM1: GC_MARK: 405, GC_SWEEP: 123, ALLOCATION: 760, EXECUTION: 1072
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>,
                                              MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // ---

    // INTEL: GC_MARK: 669, GC_SWEEP: 317, ALLOCATION: 2208, EXECUTION: 2001
    // RPI4:  GC_MARK: 5572, GC_SWEEP: 2408, ALLOCATION: 9723, EXECUTION: 8375
    // MACM1: GC_MARK: 802, GC_SWEEP: 244, ALLOCATION: 1445, EXECUTION: 2222
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerLIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 460, GC_SWEEP: 316, ALLOCATION: 2359, EXECUTION: 1971
    // RPI4:  GC_MARK: 4000, GC_SWEEP: 2416, ALLOCATION: 10094, EXECUTION: 8346
    // MACM1: GC_MARK: 787, GC_SWEEP: 245, ALLOCATION: 1494, EXECUTION: 2194
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerFIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 518, GC_SWEEP: 327, ALLOCATION: 2456, EXECUTION: 1961
    // RPI4:  GC_MARK: 4061, GC_SWEEP: 2416, ALLOCATION: 9945, EXECUTION: 8240
    // MACM1: GC_MARK: 846, GC_SWEEP: 245, ALLOCATION: 1516, EXECUTION: 2187
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<allocator::NextFitAlloca, gc::MarkerEdgeFIFO>, BIG_HEAP,
                                              BIG_LINKED_LIST, false, 0>));
}