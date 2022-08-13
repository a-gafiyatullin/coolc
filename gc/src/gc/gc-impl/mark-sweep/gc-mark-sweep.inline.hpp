#include "gc/gc-interface/gc.hpp"

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::MarkSweepGC<Allocator, MarkerType, ObjectHeaderType>::sweep()
{
    address scan = _alloca.start();
    while (scan < _alloca.end())
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;
        if (obj->is_marked())
        {
            obj->unset_marked();
        }
        else
        {
            LOG_SWEEP(scan, obj->_size);
            _alloca.free(scan);
        }
        scan = _alloca.next_object(scan);
    }
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
gc::MarkSweepGC<Allocator, MarkerType, ObjectHeaderType>::MarkSweepGC(size_t heap_size)
    : ZeroGC<Allocator, ObjectHeaderType>(heap_size), _mrkr(_alloca.start(), _alloca.end())
{
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::MarkSweepGC<Allocator, MarkerType, ObjectHeaderType>::collect()
{
    GCStatisticsScope scope(&_stat[GCStatistics<ObjectHeaderType>::GC_SWEEP]);

    {
        GCStatisticsScope scope(&_stat[GCStatistics<ObjectHeaderType>::GC_MARK]);
        _mrkr.mark_from_roots(_current_scope);
    }

    sweep();

    LOG_COLLECT();
}