#include "workloads.hpp"
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
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<gc::MarkerLIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<gc::MarkerFIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC<gc::MarkerEdgeFIFO>, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC<gc::MarkerLIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC<gc::MarkerFIFO>, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC<gc::MarkerEdgeFIFO>, VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // INTEL: GC_MARK: 0, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    RUN_TEST(
        (linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerLIFO>, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 1, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    RUN_TEST(
        (linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerFIFO>, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 0, GC_SWEEP: 0, ALLOCATION: 2, EXECUTION: 2
    RUN_TEST((
        linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerEdgeFIFO>, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));

    // ---

    // INTEL: GC_MARK: 343, GC_SWEEP: 166, ALLOCATION: 1130, EXECUTION: 976
    RUN_TEST(
        (linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerLIFO>, MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 236, GC_SWEEP: 163, ALLOCATION: 1160, EXECUTION: 1008
    RUN_TEST(
        (linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerFIFO>, MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 248, GC_SWEEP: 159, ALLOCATION: 1188, EXECUTION: 1010
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerEdgeFIFO>, MEDIUM_HEAP, MEDIUM_LINKED_LIST,
                                              false, 0>));

    // ---

    // INTEL: GC_MARK: 669, GC_SWEEP: 317, ALLOCATION: 2208, EXECUTION: 2001
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerLIFO>, BIG_HEAP, BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 460, GC_SWEEP: 316, ALLOCATION: 2359, EXECUTION: 1971
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerFIFO>, BIG_HEAP, BIG_LINKED_LIST, false, 0>));

    // INTEL: GC_MARK: 518, GC_SWEEP: 327, ALLOCATION: 2456, EXECUTION: 1961
    RUN_TEST(
        (linked_list_allocation_workload<gc::MarkSweepGC<gc::MarkerEdgeFIFO>, BIG_HEAP, BIG_LINKED_LIST, false, 0>));
}