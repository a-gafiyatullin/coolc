#include "globals.hpp"
#include <unordered_map>

#define flag_pair(flag)                                                                                                \
    {                                                                                                                  \
#flag, &flag                                                                                                   \
    }

#define int_flag_delim '='

// ---------------------------- Flags ----------------------------
#ifdef DEBUG
bool PrintAllocatedObjects = false;
bool TraceMarking = false;
bool TraceStackSlotUpdate = false;
bool TraceObjectFieldUpdate = false;
bool TraceObjectMoving = false;
bool TraceGCCycles = false;
#endif // DEBUG

bool PrintGCStatistics = false;

#ifdef LLVM_STATEPOINT_EXAMPLE
#ifdef DEBUG
bool PrintStackMaps = false;
bool TraceStackWalker = false;
#endif // DEBUG
#endif // LLVM_STATEPOINT_EXAMPLE

#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
std::string MaxHeapSize = "6Kb";
int GCAlgo = 2; // ThreadedCompactionGC
#else
int GCAlgo = 0; // ZeroGC
std::string MaxHeapSize = "384Kb";
#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE

const std::unordered_map<std::string, bool *> BoolFlags = {
#ifdef LLVM_STATEPOINT_EXAMPLE
#ifdef DEBUG
    flag_pair(PrintStackMaps),         flag_pair(TraceStackWalker),
#endif // DEBUG
#endif // LLVM_STATEPOINT_EXAMPLE
#ifdef DEBUG
    flag_pair(PrintAllocatedObjects),  flag_pair(TraceMarking),      flag_pair(TraceStackSlotUpdate),
    flag_pair(TraceObjectFieldUpdate), flag_pair(TraceObjectMoving), flag_pair(TraceGCCycles),
#endif // DEBUG
    flag_pair(PrintGCStatistics)};

const std::unordered_map<std::string, std::string *> StringFlags = {flag_pair(MaxHeapSize)};

const std::unordered_map<std::string, int *> IntFlags = {flag_pair(GCAlgo)};

// ---------------------------- Flags Settings ----------------------------
bool maybe_set(const char *arg)
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

        auto str_flag = StringFlags.find(int_flag_str.substr(0, delim_pos));
        if (str_flag != StringFlags.end())
        {
            *(str_flag->second) = int_flag_str.substr(delim_pos + 1);
            return true;
        }
    }

    return false;
}

void process_runtime_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        maybe_set(argv[i]);
    }
}