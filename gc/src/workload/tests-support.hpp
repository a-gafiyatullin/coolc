#pragma once

#include "gc/gc-interface/globals.hpp"
#include <iomanip>

#define VERY_SMALL_HEAP 192           // bytes
#define SMALL_HEAP 1024 * 1024        // 1Mb
#define MEDIUM_HEAP 512 * 1024 * 1024 // 512Mb
#define BIG_HEAP 1024 * 1024 * 1024   // 1Gbs

// test support
#define DECLARE_TEST(name) void name##_workload()

#define CYAN_COLOUR "\033[1;36m"
#define GREEN_COLOUR "\033[1;32m"
#define NO_COLOUR "\033[0m"

#define RUN_TEST(test)                                                                                                 \
    {                                                                                                                  \
        std::cout << GREEN_COLOUR << "Test: " << CYAN_COLOUR << #test << NO_COLOUR << " STARTED!" << std::endl;        \
        test();                                                                                                        \
        std::cout << GREEN_COLOUR << "Test: " << CYAN_COLOUR << #test << NO_COLOUR << " PASSED." << std::endl          \
                  << std::endl;                                                                                        \
    }