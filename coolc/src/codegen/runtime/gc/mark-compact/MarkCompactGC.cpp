#include "codegen/runtime/gc/GC.hpp"

using namespace gc;

void MarkCompactGC::collect()
{
    {
        GCStats phase(GCStats::GCPhase::MARK);

        Marker *marker = Marker::marker();
        marker->mark_from_roots();
        marker->mark_root(&_runtime_root);
    }

    GCStats phase(GCStats::GCPhase::COLLECT);
    compact();
}

// --------------------------------------- Jonkers’s threaded compactor ---------------------------------------
void ThreadedCompactionGC::thread(address *ref)
{
    // we will use size field to thread
    static_assert(sizeof(ObjectLayout::_size) == sizeof(address), "sanity");

    address obj = *ref;                                           // address of the object
    if (obj != NULL && Allocator::allocator()->is_heap_addr(obj)) // constant objects are not relocatable
    {
        ObjectLayout *hdr = (ObjectLayout *)obj;

        size_t size = hdr->_size;
        hdr->_size = (size_t)ref;

        *ref = (address)size;
    }
}

void ThreadedCompactionGC::thread_root(void *obj, address *root, const address *meta)
{
    ThreadedCompactionGC *gc = (ThreadedCompactionGC *)obj;

    // hack to check for in update
    gc->_stack_start = (address)std::min((address *)gc->_stack_start, root);
    gc->_stack_end = (address)std::max((address *)gc->_stack_end, root);

    // book suggests "thread(*fld)". Mistake?
    gc->thread(root);
}

void ThreadedCompactionGC::update(address *obj, address addr)
{
    Allocator *alloca = Allocator::allocator();

    // if it is threaded object, so all references to it was orginized to linked list
    address temp = (address)((ObjectLayout *)obj)->_size;

    // if _size field contains a heap pointer - it is a start of the list
    while (alloca->is_heap_addr(temp) || (temp >= _stack_start && temp <= _stack_end) ||
           (temp == (address)&_runtime_root))
    {
        // we points to fields
        address next = *(address *)temp;
        *(address *)temp = addr; // restore address

#ifdef DEBUG
        if (TraceStackSlotUpdate && temp >= _stack_start && temp <= _stack_end)
        {
            fprintf(stderr, "Update stack slot %p (value %p) with value %p\n", temp, next, addr);
        }
        else if (TraceObjectFieldUpdate)
        {
            fprintf(stderr, "Update object field %p (value %p) with value %p\n", temp, next, addr);
        }
#endif // DEBUG

        temp = next;
    }
    ((ObjectLayout *)obj)->_size = (size_t)temp; // restore original info
}

void ThreadedCompactionGC::update_forward_references()
{
    // thread roots
    _stack_start = (address)-1;
    _stack_end = 0;

    StackWalker::walker()->process_roots(this, &ThreadedCompactionGC::thread_root);

    if (_runtime_root)
    {
        thread(&_runtime_root);
    }

    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)Allocator::allocator();

    address free = nxtf_alloca->start();
    address scan = free;

    address heap_end = nxtf_alloca->end();

    // book suggests scan <= end. Mistake?
    while (scan < heap_end)
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
    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)Allocator::allocator();

    address free = nxtf_alloca->start();
    address scan = free;

    address heap_end = nxtf_alloca->end();

    size_t obj_size;
    while (scan < heap_end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        obj_size = obj->_size; // can be wrong for marked objects after threading

        // markword is ok, because we use _size for threading
        if (obj->is_marked())
        {
            // some threads was not processed
            update((address *)scan, free); // backward refs to scan set to free

            obj_size = obj->_size;
            assert(obj_size);

            obj->unset_marked();

            nxtf_alloca->move(obj, free);

#ifdef DEBUG
            if (TraceObjectMoving)
            {
                fprintf(stderr, "Move object %p to %p\n", obj, free);
            }
#endif // DEBUG

            free = free + obj_size;
        }

        // size field always is ok here
        scan = nxtf_alloca->next_object(scan + obj_size);
    }

    if (free <= heap_end - HEADER_SIZE)
    {
        nxtf_alloca->force_alloc_pos(free);
    }
    else
    {
        int tail = heap_end - free;
        ObjectLayout *obj = (ObjectLayout *)(free - obj_size);

        obj->_size += tail;
        obj->zero_appendix(tail);
    }
}

void ThreadedCompactionGC::compact()
{
    update_forward_references();
    update_backward_references();

    StackWalker::walker()->fix_derived_pointers();
}