add_library(gc STATIC gc-impl/gc-common.cpp
                      gc-impl/object-desc.cpp
                      gc-impl/mark-sweep/gc-mark-sweep.inline.hpp)