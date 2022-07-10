#include "workloads.hpp"

DECLARE_TEST(sanity)
{
    // last-update: 7/10/2022
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC, VERY_SMALL_HEAP>));
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // last-update: 7/10/2022
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 2, FULL_GC: 0, EXECUTION: 5
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 1631, FULL_GC: 504, EXECUTION: 2569
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 3340, FULL_GC: 1048, EXECUTION: 5297
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, BIG_HEAP, BIG_LINKED_LIST, false, 0>));
}