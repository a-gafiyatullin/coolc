#include "codegen/runtime/ObjectLayout.hpp"
#include "codegen/runtime/gc/GC.hpp"
#include <cassert>
#include <cstdio>

using namespace gc;

MarkSweepGC::MarkSweepGC(const size_t &heap_size)
{
    _allocator = new NextFitAllocator(heap_size);

    _heap_start = _allocator->start();
    _heap_end = _allocator->end();

    _marker = new ShadowStackMarkerFIFO(_heap_start, _heap_end);
}

void MarkSweepGC::collect()
{
    {
        GCStats phase(GCStats::GCPhase::MARK);
        ((ShadowStackMarkerFIFO *)_marker)->mark_from_roots();
    }

    GCStats phase(GCStats::GCPhase::COLLECT);
    sweep();
}

void MarkSweepGC::sweep()
{
    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)_allocator;

    gc_address scan = _heap_start;
    while (scan < _heap_end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        if (obj->is_marked())
        {
            obj->unset_marked();
        }
        else
        {
            _allocator->free(obj);
        }
        scan = nxtf_alloca->next_object(scan);
    }
}

MarkSweepGC::~MarkSweepGC()
{
    delete _allocator;
    delete _marker;
}