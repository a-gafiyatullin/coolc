#pragma once

#include <cstddef>

#ifdef DEBUG
extern bool PrintGCStatistics;
extern bool PrintAllocatedObjects;
#endif // DEBUG

extern size_t MaxHeapSize;

void process_runtime_args(int argc, char **argv);