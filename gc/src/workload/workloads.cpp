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
    // Battery: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
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
    // Battery: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
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
    // Battery: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    // Power supply: ALLOCATION: 0, FULL_GC: 0, EXECUTION: 0
    RUN_TEST((sanity_trivial_collect_workload<gc::MarkSweepGC, VERY_SMALL_HEAP>));
}

DECLARE_TEST(linked_list)
{
    // last-update: 7/10/2022

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 2, FULL_GC: 0, EXECUTION: 5

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 18, FULL_GC: 5, EXECUTION: 27

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Battery: ALLOCATION: 7, FULL_GC: 0, EXECUTION: 13
    // Power supply: ALLOCATION: 8, FULL_GC: 3, EXECUTION: 13
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, SMALL_HEAP, SMALL_LINKED_LIST, false, 0>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 1631, FULL_GC: 504, EXECUTION: 2569

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 9290, FULL_GC: 4312, EXECUTION: 13837

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Battery: ALLOCATION: 1330, FULL_GC: 583, EXECUTION: 2449
    // Power supply: ALLOCATION: 1278, FULL_GC: 539, EXECUTION: 2401
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, MEDIUM_HEAP, MEDIUM_LINKED_LIST, false, 0>));

    // WSL 2.0, Intel(R) Core(TM) i7-9700F CPU @ 3.00GHz, 16GB (WSL) DDR4 3200MHz (XMP), Windows 11:
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 3340, FULL_GC: 1048, EXECUTION: 5297

    // rpi4 (Broadcom BCM2711, quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz, 4GB LPDDR4), Ubuntu 20.04.4 LTS
    // clang version 10.0.0-4ubuntu1
    // ALLOCATION: 18237, FULL_GC: 8419, EXECUTION: 27290

    // Apple MacBook Air M1 2020 (Apple M1 (2.1/3.2 GHz), 8Gb LPDDR4X 4266MHz), macOS Monterey 12.4
    // Apple clang version 13.1.6 (clang-1316.0.21.2.5)
    // Battery: ALLOCATION: 2610, FULL_GC: 1171, EXECUTION: 4835
    // Power supply: ALLOCATION: 2589, FULL_GC: 1116, EXECUTION: 4778
    RUN_TEST((linked_list_allocation_workload<gc::MarkSweepGC, BIG_HEAP, BIG_LINKED_LIST, false, 0>));
}