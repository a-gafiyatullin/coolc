#include "gc/gc-interface/gc.hpp"

template <class Allocator, class MarkerType> void gc::MarkSweepGC<Allocator, MarkerType>::sweep()
{
    address scan = _alloca.start();
    while (scan < _alloca.end())
    {
        objects::ObjectHeader *obj = (objects::ObjectHeader *)scan;
        if (obj->is_marked())
        {
            obj->unset_marked();
        }
        else
        {
            LOG_SWEEP(scan, obj->_size);
            _alloca.free(scan);
        }
        scan = next_object(scan);
    }
}

template <class Allocator, class MarkerType> address gc::MarkSweepGC<Allocator, MarkerType>::next_object(address obj)
{
    objects::ObjectHeader *hdr = (objects::ObjectHeader *)obj;
    objects::ObjectHeader *possible_object = (objects::ObjectHeader *)(obj + hdr->_size);

    // search for the first object with non-zero tag
    while ((address)possible_object < _alloca.end() && possible_object->_tag == 0)
    {
        // assuming size is correct for dead objects
        possible_object = (objects::ObjectHeader *)((address)possible_object + possible_object->_size);
    }

    return (address)possible_object;
}

template <class Allocator, class MarkerType>
gc::MarkSweepGC<Allocator, MarkerType>::MarkSweepGC(size_t heap_size)
    : ZeroGC<Allocator>(heap_size), _mrkr(_alloca.start(), _alloca.end())
{
}

template <class Allocator, class MarkerType> void gc::MarkSweepGC<Allocator, MarkerType>::collect()
{
    GCStatisticsScope scope(&_stat[GCStatistics::GC_SWEEP]);

    {
        GCStatisticsScope scope(&_stat[GCStatistics::GC_MARK]);
        _mrkr.mark_from_roots(_current_scope);
    }

    sweep();

    LOG_COLLECT();
}