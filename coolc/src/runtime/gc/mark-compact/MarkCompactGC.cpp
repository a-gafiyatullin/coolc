#include "runtime/gc/GC.hpp"

using namespace gc;

void MarkCompactGC::collect()
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
           (std::any_of(_runtime_roots.begin(), _runtime_roots.end(),
                        [temp](address *e) { return e == (address *)temp; })))
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

    for (auto *r : _runtime_roots)
    {
        thread(r);
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

// --------------------------------------- One-pass compressor ---------------------------------------
void CompressorGC::compute_locations()
{
    BitMapMarker *marker = (BitMapMarker *)Marker::marker();
    _offsets.resize(marker->bits_num() / BITS_IN_BLOCK + 1);

    size_t loc = 0;
    size_t blocks_num = 0;

    size_t previous_bit = 0;
    bool in_object = false;
    for (size_t b = 0; b < marker->bits_num(); b++)
    {
        if (b % BITS_IN_BLOCK == 0)
        {
            assert(blocks_num < _offsets.size());

            _offsets[blocks_num] = loc + (in_object ? (b - previous_bit + 1) * BitMapMarker::BYTES_PER_BIT : 0);
            blocks_num++;
        }

        if (marker->is_bit_set(b))
        {
            if (in_object)
            {
                // TODO: fprintf(stderr, "Found object end = %zu\n", b);
                loc += (b - previous_bit + 1) * BitMapMarker::BYTES_PER_BIT;
                in_object = false;
            }
            else
            {
                // TODO: fprintf(stderr, "Found object start = %zu\n", b);
                in_object = true;
                previous_bit = b;
            }
        }
    }

#ifdef DEBUG
    if (TraceObjectFieldUpdate)
    {
        fprintf(stderr, "Offset table:\n");
        address heap_start = Allocator::allocator()->start();

        for (int i = 0; i < _offsets.size(); i++)
        {
            fprintf(stderr, "[%p-%p], offset[%d] = %lu\n", heap_start + i * BITS_IN_BLOCK * BitMapMarker::BYTES_PER_BIT,
                    heap_start + (i + 1) * BITS_IN_BLOCK * BitMapMarker::BYTES_PER_BIT, i, _offsets[i]);
        }
    }
#endif // DEBUG

    assert(!in_object);
}

address CompressorGC::new_address(address old)
{
    if (old == nullptr || !Allocator::allocator()->is_heap_addr(old))
    {
        return old;
    }

    address heap_start = Allocator::allocator()->start();

    size_t block_idx = (size_t)(old - heap_start) / BITS_IN_BLOCK;
    BitMapMarker *marker = (BitMapMarker *)Marker::marker();

    // calculate offset in block
    address block_start = heap_start + block_idx * BITS_IN_BLOCK;
    int used_bits = 0;

    bool in_object = false;
    size_t previous_bit = 0;
    for (size_t b = marker->byte_to_bit(block_start); b < marker->byte_to_bit(old); b++)
    {
        if (marker->is_bit_set(b))
        {
            if (in_object)
            {
                used_bits += b - previous_bit + 1;
                in_object = false;
            }
            else
            {
                in_object = true;
                previous_bit = b;
            }
        }
    }

    assert(!in_object);

    assert(block_idx < _offsets.size());
    address new_addr = heap_start + _offsets[block_idx] + used_bits * BitMapMarker::BYTES_PER_BIT;

#ifdef DEBUG
    if (TraceObjectFieldUpdate)
    {
        fprintf(stderr, "New address for object %p is %p\n", old, new_addr);
    }
#endif // DEBUG

    return new_addr;
}

void CompressorGC::update_stack_root(void *obj, address *root, const address *meta)
{
    CompressorGC *gc = (CompressorGC *)obj;
    ObjectLayout **object_ptr = (ObjectLayout **)root;

    if (!gc->_was_updated.contains(root))
    {
        *object_ptr = (ObjectLayout *)gc->new_address((address)(*object_ptr));
        gc->_was_updated.insert(root);
    }
}

void CompressorGC::update_references_relocate()
{
    // update roots on stack
    StackWalker::walker()->process_roots(this, &CompressorGC::update_stack_root);

    for (auto *r : _runtime_roots)
    {
        *r = new_address(*r);
    }

    NextFitAllocator *nxtf_alloca = (NextFitAllocator *)Allocator::allocator();
    BitMapMarker *marker = (BitMapMarker *)Marker::marker();

    address scan = nxtf_alloca->start();
    address end = nxtf_alloca->end();
    address free = nullptr;
    int size = 0;

    // traverse heap
    while (scan < end)
    {
        ObjectLayout *obj = (ObjectLayout *)scan;
        size = obj->_size;

        if (marker->is_marked(obj))
        {
            // update fields
            if (!obj->has_special_type())
            {
                int fields_cnt = obj->field_cnt();
                address *fields = obj->fields_base();
                for (int j = 0; j < fields_cnt; j++)
                {
                    ObjectLayout **object_field = (ObjectLayout **)(fields + j);
                    *object_field = (ObjectLayout *)new_address((address)*object_field);
                }
            }
            else
            {
                // special case
                if (obj->is_string())
                {
                    StringLayout *str = (StringLayout *)obj;

                    ObjectLayout **object_field = (ObjectLayout **)((address)str + HEADER_SIZE);
                    *object_field = (ObjectLayout *)new_address((address)*object_field);
                }
            }

            // and move now
            address dst = new_address((address)obj);
            nxtf_alloca->move(obj, dst);
            free = dst + size; // TODO: is it correct?
        }

        scan = nxtf_alloca->next_object(scan + size);
    }

    // handle end of the heap
    if (free <= end - HEADER_SIZE)
    {
        nxtf_alloca->force_alloc_pos(free);
    }
    else
    {
        int tail = end - free;
        ObjectLayout *obj = (ObjectLayout *)(free - size);

        obj->_size += tail;
        obj->zero_appendix(tail);
    }
}

void CompressorGC::compact()
{
    compute_locations();
    update_references_relocate();

    StackWalker::walker()->fix_derived_pointers();

    ((BitMapMarker *)Marker::marker())->clear();
    _offsets.clear();
    _was_updated.clear();
}