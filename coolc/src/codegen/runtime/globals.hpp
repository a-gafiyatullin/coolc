#pragma once

#include <cstddef>

#ifdef DEBUG
extern bool PrintGCStatistics;
extern bool PrintAllocatedObjects;
#endif // DEBUG

extern size_t MaxHeapSize;
extern size_t GCAlgo;

void process_runtime_args(int argc, char **argv);