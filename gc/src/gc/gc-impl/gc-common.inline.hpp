#pragma once

#include "gc/gc-interface/gc.hpp"

template <class Allocator> gc::ZeroGC<Allocator>::ZeroGC(size_t heap_size) : _alloca(heap_size)
{
}

template <class Allocator> address gc::ZeroGC<Allocator>::allocate(objects::Klass *klass)
{
    GCStatisticsScope scope(&_stat[GCStatistics::ALLOCATION]);

    objects::ObjectHeader *chunk = (objects::ObjectHeader *)_alloca.allocate(klass);
    if (chunk == NULL)
    {
        collect();
        chunk = (objects::ObjectHeader *)_alloca.allocate(klass);
    }
    if (chunk == NULL)
    {
        _alloca.dump();
        GCStatistics::print_gc_stats(this);
        throw std::bad_alloc();
    }

    return (address)chunk;
}