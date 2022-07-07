#include "workloads.hpp"

DECLARE_TEST(sanity)
{
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC, VERY_SMALL_HEAP>));
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));

    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));

    return 0;
}