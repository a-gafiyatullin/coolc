#pragma once

#include <string>

#ifdef DEBUG
extern bool PrintAllocatedObjects;
#endif // DEBUG

extern bool PrintGCStatistics;
extern std::string MaxHeapSize;
extern int GCAlgo;

/**
 * @brief Process arguments were passed to executable
 *
 * @param argc Arguments number
 * @param argv Arguments vector
 */
void process_runtime_args(int argc, char **argv);