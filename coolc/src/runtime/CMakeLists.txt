set(COMMON_SRC Runtime.cpp
               ObjectLayout.cpp
              
               gc/Allocator.cpp
              
               gc/Marker.cpp
               gc/StackWalker.cpp
              
               gc/GC.cpp
              
               globals.cpp
               gc/Utils.cpp)

if(GCTYPE STREQUAL "LLVM_SHADOW_STACK" OR GCTYPE STREQUAL "LLVM_STATEPOINT_EXAMPLE")
    set(GC_SRC gc/mark-sweep/MarkSweepGC.cpp
               gc/mark-compact/MarkCompactGC.cpp)
endif()

if(GCTYPE STREQUAL "LLVM_STATEPOINT_EXAMPLE")
    set(STACKMAP_SRC gc/stack-map/StackMap.cpp)
endif()

add_library(cool-rt SHARED ${COMMON_SRC} ${GC_SRC} ${STACKMAP_SRC})