#include "Allocator.hpp"

using namespace gc;

Allocator::Allocator(const size_t &size)
    : _size(size)
#ifdef DEBUG
      ,
      _allocated_size(0), _freed_size(0)
#endif // DEBUG
{
    _start = (gc_address)malloc(_size);
    if (_start == NULL)
    {
        exit_with_error("cannot allocate memory for heap!");
    }
    _end = _start + _size;
    _pos = _start;
}

void Allocator::exit_with_error(const char *error)
{
#ifdef DEBUG
    dump();
#endif // DEBUG

    // TODO: one day we should handle it with exceptions
    fprintf(stderr, "\n%s\n", error);
    exit(-1);
}

Allocator::~Allocator()
{
#ifdef DEBUG
    dump();
#endif // DEBUG
    std::free(_start);
}

#ifdef DEBUG
void Allocator::dump()
{
    fprintf(stderr, "Allocated bytes: %lu\nFreed bytes: %lu", _allocated_size, _freed_size);
}
#endif // DEBUG

ObjectLayout *Allocator::allocate(int tag, size_t size, void *disp_tab)
{
    auto *object = allocate_inner(tag, size, disp_tab);
#ifdef DEBUG
    _allocated_size += size;
#endif // DEBUG
    return object;
}

ObjectLayout *Allocator::allocate_inner(int tag, size_t size, void *disp_tab)
{
    if (_pos + size >= _start + _size)
    {
        exit_with_error("cannot allocate memory for object!");
    }

    gc_address object = _pos;
    _pos += size;

    ObjectLayout *obj_header = (ObjectLayout *)object;
    obj_header->_mark = MarkWordUnsetValue;
    obj_header->_size = size;
    obj_header->_tag = tag;
    obj_header->_dispatch_table = disp_tab;

    return (ObjectLayout *)object;
}

void Allocator::free(ObjectLayout *obj)
{
#ifdef DEBUG
    _freed_size += obj->_size;
#endif // DEBUG
    free_inner(obj);
}

// -------------------------------------------- NextFitAllocator --------------------------------------------
NextFitAllocator::NextFitAllocator(const size_t &size) : Allocator(size)
{
    // create an artificial object with tag 0 and size heap_size
    force_alloc_pos(_start);
}

ObjectLayout *NextFitAllocator::allocate_inner(int tag, size_t size, void *disp_tab)
{
    // try to find suitable chunk of the memeory
    // compact chunks by the way
    ObjectLayout *chunk = NULL;
    // assume that we allocate memory consequently between collections
    ObjectLayout *current_chunk = (ObjectLayout *)(_pos ? _pos : _start);

    size_t current_chunk_size = 0;
    while ((gc_address)current_chunk < _end)
    {
        if (current_chunk->_tag != 0)
        {
            // found allocated object

            // merge previous chunks, but it is not ours chunk
            if (current_chunk_size != 0 && chunk != NULL)
            {
                chunk->_size = current_chunk_size;
            }
            chunk = NULL;
        }
        else if (current_chunk->_tag == 0 && chunk == NULL)
        {
            // remember the first free chunk
            if (_pos == NULL)
            {
                _pos = (gc_address)current_chunk;
            }
            // first free chunk
            chunk = current_chunk;
            current_chunk_size = current_chunk->_size;
        }
        else if (current_chunk->_tag == 0)
        {
            // remember the first free chunk
            if (_pos == NULL)
            {
                _pos = (gc_address)current_chunk;
            }
            // not the first free contiguous chunk
            // merge previous chunks
            current_chunk_size += current_chunk->_size;
            chunk->_size = current_chunk_size;
        }

        // enough memory?
        if (current_chunk_size >= size)
        {
            break;
        }

        // go to next chunk
        current_chunk = (ObjectLayout *)((gc_address)current_chunk + current_chunk->_size);
    }

    if (!chunk || chunk->_size < size)
    {
        return NULL;
    }

    int appendix_size = 0;

    if (current_chunk_size - size < HEADER_SIZE)
    {
        appendix_size = current_chunk_size - size;
        size = current_chunk_size; // allign allocation for correct heap interation
        // this headen fields always will be zero, so will not affeсt gc
    }

    if (current_chunk_size - size >= HEADER_SIZE)
    {
        // adjust next chunk
        // at least have memory for the header

        ObjectLayout *next_free = (ObjectLayout *)((gc_address)chunk + size);
        next_free->set_unused(current_chunk_size - size);
        _pos = (gc_address)next_free;
    }

    chunk->_mark = MarkWordUnsetValue;
    chunk->_size = size;
    chunk->_tag = tag;
    chunk->_dispatch_table = disp_tab;

    if (appendix_size != 0)
    {
        chunk->zero_appendix(appendix_size);
    }

    return chunk;
}

void NextFitAllocator::free_inner(ObjectLayout *obj)
{
    obj->unset_marked();

#ifdef DEBUG
    obj->zero_fields(0xBAD);
#endif // DEBUG

    obj->_tag = UnusedTag;

    // save size for allocation
    assert(obj->_size != 0);

    // update the first free pos
    if ((gc_address)obj < _pos)
    {
        _pos = (gc_address)obj;
    }
}

void NextFitAllocator::move(const ObjectLayout *src, gc_address dst)
{
    if (dst != (gc_address)src)
    {
        assert(dst >= (gc_address)src + src->_size || dst + src->_size <= (gc_address)src);
        memcpy(dst, (gc_address)src, src->_size);
    }
}

void NextFitAllocator::force_alloc_pos(gc_address pos)
{
    // create an artificial object with tag 0 and size heap_size
    ObjectLayout *aobj = (ObjectLayout *)pos;
    aobj->set_unused(_end - pos);

    _pos = pos;
}

gc_address NextFitAllocator::next_object(gc_address obj)
{
    ObjectLayout *hdr = (ObjectLayout *)obj;
    assert(hdr->_size != 0);
    ObjectLayout *possible_object = (ObjectLayout *)(obj + hdr->_size);

    // search for the first object with non-zero tag
    while ((gc_address)possible_object < _end && possible_object->_tag == 0)
    {
        // assuming size is correct for dead objects
        possible_object = (ObjectLayout *)((gc_address)possible_object + possible_object->_size);
    }

    return (gc_address)possible_object;
}