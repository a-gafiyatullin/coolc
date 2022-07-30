add_library(gc STATIC gc-impl/gc-common.cpp
                      gc-impl/alloca.cpp
                      gc-impl/object.cpp
                      gc-impl/mark.cpp)