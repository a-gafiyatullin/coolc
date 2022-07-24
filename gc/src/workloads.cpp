#include "workload/workloads.hpp"
#include "gc/gc-interface/gc.hpp"
#include <cstring>

#define GLOBAL_VAR_NAME(var) #var

#define CHECK_VAR_AND_SET(str, var, val)                                                                               \
    if (!strcmp(str, GLOBAL_VAR_NAME(var)))                                                                            \
    {                                                                                                                  \
        var = val;                                                                                                     \
    }

void process_args(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    process_args(argc, argv);

    sanity_workload();
    linked_list_workload();

    return 0;
}

void process_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        // clang-format off
        CHECK_VAR_AND_SET(argv[i], LOGALOC, true)
        else CHECK_VAR_AND_SET(argv[i], LOGMARK, true)
        else CHECK_VAR_AND_SET(argv[i], LOGSWEEP, true)
        else CHECK_VAR_AND_SET(argv[i], LOGGING, true)
        else CHECK_VAR_AND_SET(argv[i], ZEROING, true)
        // clang-format on
    }
}