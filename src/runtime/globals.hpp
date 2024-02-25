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
extern bool TraceVerifyOops;
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
    SEMISPACE_COPYING_GC,

    GcTypeNumber
};

/**
 * @brief Align size to n-word boundary
 *
 * @param byte Number of bytes
 * @param words Words number
 * @return size_t Aligned number of bytes
 */
size_t align(size_t byte, int words = 1);

/**
 * @brief Check if size is aligned to n-word boundary
 *
 * @param byte Number of bytes
 * @param words Words number
 * @return true if size is aligned to n-word boundary
 * @return false if size is not aligned to n-word boundary
 */
bool is_aligned(size_t byte, int words = 1);

/**
 * @brief Process arguments were passed to executable
 *
 * @param argc Arguments number
 * @param argv Arguments vector
 */
void process_runtime_args(int argc, char **argv);
