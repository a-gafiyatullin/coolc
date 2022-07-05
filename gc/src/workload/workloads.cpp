#include "workloads.hpp"

int sanity_workloads()
{
    sanity_workload_trivial<gc::ZeroGC, VERY_SMALL_HEAP>();
    sanity_workload_trivial<gc::MarkSweepGC, VERY_SMALL_HEAP>();

    sanity_workload_trivial_collect<gc::MarkSweepGC, VERY_SMALL_HEAP>();

    return 0;
}