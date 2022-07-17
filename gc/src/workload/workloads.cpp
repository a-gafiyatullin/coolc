#include "workloads.hpp"

DECLARE_TEST(sanity)
{
    // last-update: 7/10/2022

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_workload<gc::ZeroGC, VERY_SMALL_HEAP>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // last-update: 7/10/2022

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 0, ALLOCATION: 1, EXECUTION: 3

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 6, ALLOCATION: 11, EXECUTION: 9

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: FULL_GC: 3, ALLOCATION: 3, EXECUTION: 8
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 502, ALLOCATION: 1158, EXECUTION: 947

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 4188, ALLOCATION: 4813, EXECUTION: 4477

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: FULL_GC: 558, ALLOCATION: 744, EXECUTION: 1112
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 995, ALLOCATION: 2353, EXECUTION: 1851

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // FULL_GC: 8243, ALLOCATION: 9805, EXECUTION: 8259

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Power supply: FULL_GC: 1135, ALLOCATION: 1441, EXECUTION: 2226
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, BIG_HEAP, BIG_LINKED_LIST, false, 0>));
}