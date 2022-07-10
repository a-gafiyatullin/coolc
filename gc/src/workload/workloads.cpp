#include "workloads.hpp"

DECLARE_TEST(sanity)
{
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
    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP):
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 4485, FULL_GC: 0, EXECUTION: 4488
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, SMALL_HEAP, SMALL_LINKED_LIST>));
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, MEDIUM_HEAP, MEDIUM_LINKED_LIST>));
}