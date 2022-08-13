#pragma once

#include "gc/gc-interface/globals.hpp"
#include <iomanip>

#define VERY_SMALL_HEAP 192                                   // bytes
#define SMALL_HEAP 1024 * 1024                                // 1Mb
#define MEDIUM_HEAP 512 * 1024 * 1024                         // 512Mb
#define BIG_HEAP 1024 * 1024 * 1024                           // 1Gbs
#define VERY_BIG_HEAP (size_t)2 * 1024 * 1024 * 1024          // 2Gbs
#define EXTREMELY_BIG_HEAP (size_t)6 * 1024 * 1024 * 1024     // 6Gbs
#define RIDICULOUSLY_BIG_HEAP (size_t)10 * 1024 * 1024 * 1024 // 10Gbs

// test support
#define CYAN_COLOUR "\033[1;36m"
#define GREEN_COLOUR "\033[1;32m"
#define NO_COLOUR "\033[0m"

#define pre_test(test, heap_size)                                                                                      \
    std::cout << GREEN_COLOUR << "Test: " << CYAN_COLOUR << #test << " with " << #heap_size << " bytes" << NO_COLOUR   \
              << " STARTED!" << std::endl;

#define post_test(test, heap_size)                                                                                     \
    std::cout << GREEN_COLOUR << "Test: " << CYAN_COLOUR << #test << " with " << #heap_size << " bytes" << NO_COLOUR   \
              << " PASSED." << std::endl                                                                               \
              << std::endl;

#define RUN_TEST(test, heap_size)                                                                                      \
    {                                                                                                                  \
        pre_test(test, heap_size);                                                                                     \
        test(heap_size);                                                                                               \
        post_test(test, heap_size);                                                                                    \
    }

#define RUN_TEST1(test, heap_size, arg1)                                                                               \
    {                                                                                                                  \
        pre_test(test, heap_size);                                                                                     \
        test(heap_size, arg1);                                                                                         \
        post_test(test, heap_size);                                                                                    \
    }