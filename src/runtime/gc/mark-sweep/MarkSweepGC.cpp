#include "runtime/gc/GC.hpp"

using namespace gc;

void MarkSweepGC::collect()
{
    {
        GCStats phase(GCStats::GCPhase::MARK);

        Marker *marker = Marker::marker();
        marker->mark_from_roots();
        for (auto *r : _runtime_roots)
        {
            marker->mark_root(r);
        }
    }

    GCStats phase(GCStats::GCPhase::COLLECT);
    sweep();
}

void MarkSweepGC::sweep()
{
    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)Allocator::allocator();

    address scan = nxtf_alloca->start();
    address heap_end = nxtf_alloca->end();
    while (scan < heap_end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        if (obj->is_marked())
        {
            obj->unset_marked();
        }
        else
        {
            nxtf_alloca->free(obj);
        }
        scan = nxtf_alloca->next_object(scan + obj->_size); // it's ok to use size after free
    }
}