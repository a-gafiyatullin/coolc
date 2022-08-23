#include "globals.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#define KB 1024
#define MB 1024 * KB

#define flag_name(flag) #flag
#define int_flag_delim '='

// ---------------------------- Flags ----------------------------
#ifdef DEBUG
bool PrintAllocatedObjects = false;
#endif // DEBUG

bool PrintGCStatistics = false;

size_t MaxHeapSize = 6 * KB;
size_t GCAlgo = 1; // gc::GC::MARKSWEEPGC

const std::unordered_map<std::string, bool *> BoolFlags = {
    {flag_name(PrintGCStatistics), &PrintGCStatistics}
#ifdef DEBUG
    ,
    {flag_name(PrintAllocatedObjects), &PrintAllocatedObjects},
#endif // DEBUG
};

const std::unordered_map<std::string, size_t *> IntFlags = {{flag_name(MaxHeapSize), &MaxHeapSize},
                                                            {flag_name(GCAlgo), &GCAlgo}};

// ---------------------------- Flags Settings ----------------------------
bool maybe_set(const char *arg, const char *flag_name)
{
    auto bool_flags = BoolFlags.find(arg + 1);
    if (bool_flags != BoolFlags.end())
    {
        *(bool_flags->second) = arg[0] == '+';
        return true;
    }

    std::string int_flag_str = arg;
    int delim_pos = int_flag_str.find(int_flag_delim);
    if (delim_pos != std::string::npos)
    {
        auto int_flag = IntFlags.find(int_flag_str.substr(0, delim_pos));
        if (int_flag != IntFlags.end())
        {
            *(int_flag->second) = std::stoi(int_flag_str.substr(delim_pos + 1));
            return true;
        }
    }

    return false;
}

#define check_flag(flag)                                                                                               \
    if (maybe_set(argv[i], #flag))                                                                                     \
    {                                                                                                                  \
        continue;                                                                                                      \
    }

void process_runtime_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
#ifdef DEBUG
        check_flag(PrintGCStatistics);
        check_flag(PrintAllocatedObjects);
#endif // DEBUG
        check_flag(MaxHeapSize);
    }
}