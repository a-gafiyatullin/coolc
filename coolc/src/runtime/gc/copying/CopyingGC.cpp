#include "runtime/gc/Allocator.hpp"
#include "runtime/gc/GC.hpp"

using namespace gc;

void SemispaceCopyingGC::collect()
{
    GCStats phase(GCStats::GCPhase::COLLECT); // don't have explicit mark phase

    SemispaceNextFitAllocator *alloca = (SemispaceNextFitAllocator *)Allocator::allocator();
    alloca->flip();

    address scan = alloca->tospace();
    _free = scan;

    assert(is_alligned((size_t)scan));

    // traverse stack roots
    StackWalker::walker()->process_roots(this, &SemispaceCopyingGC::update_stack_root, true);

    // traverse runtime stack roots
    for (auto *r : _runtime_roots)
    {
        process(r);
    }

    size_t size = 0;

    // do bfs
    while (scan != _free)
    {
        assert(scan < alloca->end());
        assert(_free <= alloca->end());

        ObjectLayout *obj = (ObjectLayout *)scan;
        size = obj->_size;

        assert(size <= alloca->end() - alloca->start());

        // process fields
        if (!obj->has_special_type())
        {
            int fields_cnt = obj->field_cnt();
            address *fields = obj->fields_base();
            for (int j = 0; j < fields_cnt; j++)
            {
                process(fields + j);
            }
        }
        else
        {
            // special case
            if (obj->is_string())
            {
                process((address *)((address)obj + HEADER_SIZE));
            }
        }

        scan = scan + size;
    }

    // handle end of the heap
    if (_free <= alloca->end() - HEADER_SIZE)
    {
        alloca->force_alloc_pos(_free);
    }
    else
    {
        int tail = alloca->end() - _free;
        ObjectLayout *obj = (ObjectLayout *)(_free - size);

        obj->_size += tail;
        obj->zero_appendix(tail);
    }

    StackWalker::walker()->fix_derived_pointers();
}

void SemispaceCopyingGC::update_stack_root(void *obj, address *root, const address *meta)
{
#ifdef DEBUG
    if (TraceStackSlotUpdate)
    {
        fprintf(stderr, "Updated root %p, value before = %p\n", root, *root);
    }
#endif // DEBUG
    ((SemispaceCopyingGC *)obj)->process(root);
}

void SemispaceCopyingGC::process(address *root)
{
#ifdef DEBUG
    if (TraceObjectFieldUpdate)
    {
        fprintf(stderr, "Before update *%p = %p\n", root, *root);
    }
#endif // DEBUG

    // prevent update of the already updated fields
    // inverted is_heap_addr result because now old objects are out of heap,
    // but it is not a constant
    if (*root && !Allocator::allocator()->is_heap_addr(*root) &&
        ((SemispaceNextFitAllocator *)Allocator::allocator())->is_orig_heap_addr(*root))
    {
        *root = forward(*(ObjectLayout **)root);
    }

#ifdef DEBUG
    if (TraceObjectFieldUpdate)
    {
        fprintf(stderr, "Updated %p with %p\n", root, *root);
    }
#endif // DEBUG

    assert(!*root || Allocator::allocator()->is_heap_addr(*root) ||
           !((SemispaceNextFitAllocator *)Allocator::allocator())->is_orig_heap_addr(*root));
}

address SemispaceCopyingGC::SemispaceCopyingGC::forward(ObjectLayout *fromref)
{
    if (fromref->is_marked())
    {
        return (address)fromref->_size;
    }

    return copy(fromref);
}

void SemispaceCopyingGC::set_forwarding_address(ObjectLayout *old, address newloc)
{
    old->set_marked();
    old->_size = (size_t)newloc;
}

address SemispaceCopyingGC::copy(ObjectLayout *old)
{
    assert((address)old >= ((SemispaceNextFitAllocator *)Allocator::allocator())->fromspace());

    address toref = _free;
    // size is ok here
    _free = _free + old->_size;

    ((SemispaceNextFitAllocator *)Allocator::allocator())->move(old, toref);

    // now we are ready to destruct size in old object
    set_forwarding_address(old, toref);

    return toref;
}