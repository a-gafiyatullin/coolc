#pragma once

#include <iomanip>

// TODO: some problems with very small heaps?
#define VERY_SMALL_HEAP 1024

// test support
#define DECLARE_TEST(name) void name##_workload()
#define DECLARE_TEMPLATE_TEST(name) template <class GCType, int heap_size> DECLARE_TEST(name)

#define CYAN_COLOUR "\033[1;36m"
#define GREEN_COLOUR "\033[1;32m"
#define NO_COLOUR "\033[0m"

#define RUN_TEST(test)                                                                                                 \
    {                                                                                                                  \
        std::cout << "Test: " << CYAN_COLOUR << #test << NO_COLOUR << " STARTED!" << std::endl;                        \
        test();                                                                                                        \
        std::cout << "Test: " << CYAN_COLOUR << #test << NO_COLOUR << " PASSED." << std::endl << std::endl;            \
    }