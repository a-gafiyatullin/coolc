add_library(cool-rt SHARED Runtime.cpp
                           ObjectLayout.cpp
                           
                           gc/Allocator.cpp
                           gc/GC.cpp
                           gc/Marker.cpp
                           
                           gc/mark-sweep/MarkSweepGC.cpp
                           gc/mark-compact/MarkCompactGC.cpp

                           globals.cpp
                           
                           gc/Utils.cpp)