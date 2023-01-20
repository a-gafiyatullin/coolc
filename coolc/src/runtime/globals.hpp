#pragma once

#include <string>

#define address char *

#ifdef DEBUG
extern bool PrintAllocatedObjects;
extern bool TraceMarking;
extern bool TraceStackSlotUpdate;
extern bool TraceObjectFieldUpdate;
extern bool TraceObjectMoving;
extern bool TraceGCCycles;
#endif // DEBUG

extern bool PrintGCStatistics;
extern std::string MaxHeapSize;
extern int GCAlgo;

#ifdef LLVM_STATEPOINT_EXAMPLE
#ifdef DEBUG
extern bool PrintStackMaps;
extern bool TraceStackWalker;
#endif // DEBUG
#endif // LLVM_STATEPOINT_EXAMPLE

enum GcType
{
    ZEROGC, // Dummy GC that don't really collect garbage
    MARKSWEEPGC,
    THREADED_MC_GC,
    COMPRESSOR_GC,

    GcTypeNumber
};

/**
 * @brief Allign size to sizeof(address) boundary
 *
 * @param byte Number of bytes
 * @return size_t Alligned number of bytes
 */
size_t allign(size_t byte);

/**
 * @brief Check if size is alligned to sizeof(address) boundary
 *
 * @param byte Number of bytes
 * @return true if size is alligned to sizeof(address) boundary
 * @return false if size is not alligned to sizeof(address) boundary
 */
bool is_alligned(size_t byte);

/**
 * @brief Process arguments were passed to executable
 *
 * @param argc Arguments number
 * @param argv Arguments vector
 */
void process_runtime_args(int argc, char **argv);