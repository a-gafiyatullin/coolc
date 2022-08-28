#include "codegen/runtime/gc/GC.hpp"

using namespace gc;

MarkCompactGC::MarkCompactGC(const size_t &heap_size)
{
    _allocator = new NextFitAllocator(heap_size);

    _heap_start = _allocator->start();
    _heap_end = _allocator->end();

    _marker = new ShadowStackMarkerFIFO(_heap_start, _heap_end);
}

void MarkCompactGC::collect()
{
    {
        GCStats phase(GCStats::GCPhase::MARK);
        ((ShadowStackMarkerFIFO *)_marker)->mark_from_roots();
        _marker->mark_runtime_root(_runtime_root);
    }

    GCStats phase(GCStats::GCPhase::COLLECT);
    compact();
}

MarkCompactGC::~MarkCompactGC()
{
    delete _allocator;
    delete _marker;
}

// --------------------------------------- Jonkers’s threaded compactor ---------------------------------------
void ThreadedCompactionGC::thread(address *ref)
{
    // we will use size field to thread
    static_assert(sizeof(ObjectLayout::_size) == sizeof(address), "sanity");

    address obj = *ref;                               // address of the object
    if (obj != NULL && _allocator->is_heap_addr(obj)) // constant objects are not relocatable
    {
        ObjectLayout *hdr = (ObjectLayout *)obj;

        size_t size = hdr->_size;
        hdr->_size = (size_t)ref;

        *ref = (address)size;
    }
}

void ThreadedCompactionGC::update(address *obj, address addr)
{
    // if it is threaded object, so all references to it was orginized to linked list
    address temp = (address)((ObjectLayout *)obj)->_size;

    // if _size field contains a heap pointer - it is a start of the list
    while (_allocator->is_heap_addr(temp) || (temp >= _stack_start && temp <= _stack_end) ||
           (temp == (address)&_runtime_root))
    {
        // we points to fields
        address next = *(address *)temp;
        *(address *)temp = addr; // restore address
        temp = next;
    }
    ((ObjectLayout *)obj)->_size = (size_t)temp; // restore original info
}

void ThreadedCompactionGC::update_forward_references()
{
    // thread roots
    StackEntry *r = llvm_gc_root_chain;

    _stack_start = (address)-1;
    _stack_end = 0;

    while (r)
    {
        assert(r->_map->_num_meta == 0); // we don't use metadata

        int num_roots = r->_map->_num_roots;
        for (int i = 0; i < num_roots; i++)
        {
            address *root = (address *)(r->_roots + i);

            // hack to check for in update
            _stack_start = (address)std::min((address *)_stack_start, root);
            _stack_end = (address)std::max((address *)_stack_end, root);

            // book suggests "thread(*fld)". Mistake?
            thread(root);
        }

        r = r->_next;
    }

    if (_runtime_root)
    {
        thread(&_runtime_root);
    }

    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)_allocator;

    address free = _heap_start;
    address scan = free;

    // book suggests scan <= end. Mistake?
    while (scan < _heap_end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        size_t obj_size = obj->_size; // can be wrong for marked objects after threading

        // markword is ok, because we use _size for threading
        if (obj->is_marked())
        {
            update((address *)scan, free); // forward refs to scan set to free

            obj_size = obj->_size;
            assert(obj_size);

            if (!obj->has_special_type())
            {
                int fields_cnt = obj->field_cnt();
                address *fields = obj->fields_base();
                for (int j = 0; j < fields_cnt; j++)
                {
                    thread(fields + j);
                }
            }
            else
            {
                // special case
                if (obj->is_string())
                {
                    StringLayout *str = (StringLayout *)obj;
                    thread((address *)((address)str + HEADER_SIZE));
                }
            }

            free = free + obj_size;
        }

        // size field always is ok here
        scan = nxtf_alloca->next_object(scan + obj_size);
    }
}

void ThreadedCompactionGC::update_backward_references()
{
    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)_allocator;

    address free = _heap_start;
    address scan = free;

    while (scan < _heap_end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        size_t obj_size = obj->_size; // can be wrong for marked objects after threading

        // markword is ok, because we use _size for threading
        if (obj->is_marked())
        {
            // some threads was not processed
            update((address *)scan, free); // backward refs to scan set to free

            obj_size = obj->_size;
            assert(obj_size);

            obj->unset_marked();

            nxtf_alloca->move(obj, free);

            free = free + obj_size;
        }

        // size field always is ok here
        scan = nxtf_alloca->next_object(scan + obj_size);
    }

    nxtf_alloca->force_alloc_pos(free);
}

void ThreadedCompactionGC::compact()
{
    update_forward_references();
    update_backward_references();
}