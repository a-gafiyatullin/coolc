#include "workloads.hpp"

DECLARE_TEST(sanity)
{
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, SMALL_HEAP, SMALL_LINKED_LIST>));
}