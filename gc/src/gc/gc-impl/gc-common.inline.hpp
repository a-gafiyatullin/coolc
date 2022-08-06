#pragma once

#include "gc/gc-interface/gc.hpp"

// --------------------------------------- StackRecord ---------------------------------------
template <class ObjectHeaderType>
gc::StackRecord<ObjectHeaderType>::StackRecord(GC<ObjectHeaderType> *gc) : _gc(gc), _parent(NULL)
{
    gc->_current_scope = this;
}

template <class ObjectHeaderType> gc::StackRecord<ObjectHeaderType>::~StackRecord()
{
    _gc->_current_scope = _parent;
}

template <class ObjectHeaderType>
gc::StackRecord<ObjectHeaderType>::StackRecord(StackRecord *parent) : StackRecord(parent->_gc)
{
    _parent = parent;
}

// --------------------------------------- GCStatisticsScope ---------------------------------------
template <class ObjectHeaderType>
const char *
    gc::GCStatistics<ObjectHeaderType>::GCStatisticsName[gc::GCStatistics<ObjectHeaderType>::GCStatisticsTypeAmount] = {
        "GC_MARK", "GC_MAIN_PHASE", "ALLOCATION", "EXECUTION"};

template <class ObjectHeaderType>
void gc::GCStatistics<ObjectHeaderType>::print(GCStatisticsType type, const GCStatistics &stat, long long sub,
                                               const char *delim)
{
    std::cout << GCStatisticsName[type] << ": " << stat.time() - sub << delim;
}

template <class ObjectHeaderType> void gc::GCStatistics<ObjectHeaderType>::print_gc_stats(GC<ObjectHeaderType> *gc)
{
    long long sum = 0;
    for (int i = GCStatistics::GC_MARK; i < gc::GCStatistics<ObjectHeaderType>::GCStatisticsTypeAmount - 1; i++)
    {
        auto stat_i = (gc::GCStatistics<ObjectHeaderType>::GCStatisticsType)i;
        print(stat_i, gc->stat(stat_i), sum, ", ");

        sum = gc->stat(stat_i).time();
    }

    auto last_stat = (gc::GCStatistics<ObjectHeaderType>::GCStatisticsType)(
        gc::GCStatistics<ObjectHeaderType>::GCStatisticsTypeAmount - 1);
    print(last_stat, gc->stat(last_stat), sum, "\n");
}

template <class ObjectHeaderType>
gc::GCStatisticsScope<ObjectHeaderType>::GCStatisticsScope(GCStatistics<ObjectHeaderType> *stat)
    : _stat(stat), _start(duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch()))
{
}

template <class ObjectHeaderType> void gc::GCStatisticsScope<ObjectHeaderType>::flush()
{
    precision now = duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch());
    _stat->add_time(now - _start);
    _start = now;
}

template <class ObjectHeaderType> gc::GCStatisticsScope<ObjectHeaderType>::~GCStatisticsScope()
{
    _stat->add_time(duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch()) - _start);
}

// --------------------------------------- GC ---------------------------------------
template <class ObjectHeaderType>
gc::GC<ObjectHeaderType>::GC() : _exec(&_stat[GCStatistics<ObjectHeaderType>::EXECUTION])
{
}

template <class ObjectHeaderType> gc::GC<ObjectHeaderType>::~GC()
{
    _exec.flush();
    GCStatistics<ObjectHeaderType>::print_gc_stats(this);
}

// --------------------------------------- ZeroGC ---------------------------------------
template <template <class> class Allocator, class ObjectHeaderType>
gc::ZeroGC<Allocator, ObjectHeaderType>::ZeroGC(size_t heap_size) : _alloca(heap_size)
{
}

template <template <class> class Allocator, class ObjectHeaderType>
address gc::ZeroGC<Allocator, ObjectHeaderType>::allocate(objects::Klass<ObjectHeaderType> *klass)
{
    GCStatisticsScope scope(&_stat[GCStatistics<ObjectHeaderType>::ALLOCATION]);

    ObjectHeaderType *chunk = (ObjectHeaderType *)_alloca.allocate(klass);
    if (chunk == NULL)
    {
        collect();
        chunk = (ObjectHeaderType *)_alloca.allocate(klass);
    }
    if (chunk == NULL)
    {
        _alloca.dump();
        GCStatistics<ObjectHeaderType>::print_gc_stats(this);
        throw std::bad_alloc();
    }

    return (address)chunk;
}