#include "gc/gc-interface/gc.hpp"

void gc::MarkSweepGC::sweep()
{
    address scan = _heap_start;
    while (scan < _heap_end)
    {
        objects::ObjectHeader *obj = (objects::ObjectHeader *)scan;
        if (obj->is_marked())
        {
            obj->unset_marked();
        }
        else
        {
            free(scan);
        }
        scan = next_object(scan);
    }
}

void gc::MarkSweepGC::free(address obj)
{
    // memory chunk is free if tag of the object starts from this address is 0

    objects::ObjectHeader *hdr = (objects::ObjectHeader *)obj;
    hdr->unset_marked();
    if (_need_zeroing)
    {
        memset(obj + sizeof(objects::ObjectHeader), 0, hdr->_size - sizeof(objects::ObjectHeader));
    }
    hdr->_tag = 0;
    // save size for allocation
    assert(hdr->_size != 0);
}

address gc::MarkSweepGC::next_object(address obj)
{
    objects::ObjectHeader *hdr = (objects::ObjectHeader *)obj;
    objects::ObjectHeader *possible_object = (objects::ObjectHeader *)(obj + hdr->_size);

    // search for the first object with non-zero tag
    while (possible_object->_tag == 0 && (address)possible_object < _heap_end)
    {
        // assuming size is correct for dead objects
        possible_object = (objects::ObjectHeader *)((address)possible_object + possible_object->_size);
    }

    return (address)possible_object;
}

gc::MarkSweepGC::MarkSweepGC(size_t heap_size, bool need_zeroing)
    : ZeroGC(heap_size, need_zeroing), _heap_end(_heap_start + _heap_size)
{
    // create an artificial object with tag 0 and size heap_size
    objects::ObjectHeader *aobj = (objects::ObjectHeader *)_heap_start;
    aobj->set_unused(heap_size);
}

address gc::MarkSweepGC::find_free_chunk(size_t size)
{
    // try to find suitable chunk of the memeory
    // compact chunks by the way
    objects::ObjectHeader *possible_chunk = NULL;
    objects::ObjectHeader *current_chunk = (objects::ObjectHeader *)_heap_start;
    size_t current_chunk_size = 0;
    while ((address)current_chunk < _heap_end)
    {
        if (current_chunk->_tag != 0)
        {
            // found allocated object

            // merge previous chunks, but it is not ours chunk
            if (current_chunk_size != 0 && possible_chunk != NULL)
            {
                possible_chunk->_size = current_chunk_size;
            }
            possible_chunk = NULL;
        }
        else if (current_chunk->_tag == 0 && possible_chunk == NULL)
        {
            // first free chunk
            possible_chunk = current_chunk;
            current_chunk_size = current_chunk->_size;
        }
        else if (current_chunk->_tag == 0)
        {
            // not the first free contiguous chunk
            // merge previous chunks
            current_chunk_size += current_chunk->_size;
            possible_chunk->_size = current_chunk_size;
        }

        // enough memory?
        if (current_chunk_size >= size)
        {
            break;
        }

        // go to next chunk
        current_chunk = (objects::ObjectHeader *)((address)current_chunk + current_chunk->_size);
    }

    return (address)possible_chunk;
}

address gc::MarkSweepGC::allocate(objects::Klass *klass)
{
    size_t obj_size = klass->size();

    objects::ObjectHeader *possible_chunk = (objects::ObjectHeader *)find_free_chunk(obj_size);
    if (!possible_chunk)
    {
        collect();
    }
    possible_chunk = (objects::ObjectHeader *)find_free_chunk(obj_size);
    if (possible_chunk == NULL)
    {
        throw std::bad_alloc();
    }

    size_t current_chunk_size = possible_chunk->_size;

    address object = (address)possible_chunk;
    if (current_chunk_size - obj_size < sizeof(objects::ObjectHeader))
    {
        obj_size = current_chunk_size; // allign allocation for correct heap interation
        // this headen fields always will be zero, so will not affeсt gc
    }

    objects::ObjectHeader *obj_header = (objects::ObjectHeader *)object;
    obj_header->_mark = 0;
    obj_header->_size = obj_size;
    obj_header->_tag = klass->type();

    if (_need_zeroing)
    {
        memset(object + sizeof(objects::ObjectHeader), 0, obj_size);
    }

    // adjust next chunk
    // at least have memory for the header
    if (current_chunk_size - obj_size >= sizeof(objects::ObjectHeader))
    {
        objects::ObjectHeader *next_free = (objects::ObjectHeader *)((address)possible_chunk + obj_size);
        next_free->set_unused(current_chunk_size - obj_size);
    }

    return object;
}

void gc::MarkSweepGC::collect()
{
    _mrkr.mark_from_roots(_current_scope);
    sweep();
}