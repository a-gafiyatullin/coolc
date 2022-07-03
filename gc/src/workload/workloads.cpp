#include "workloads.hpp"

int sanity_workloads()
{
    sanity_workload<gc::ZeroGC, VERY_SMALL_HEAP>();

    return 0;
}