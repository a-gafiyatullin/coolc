#include "Allocator.hpp"
#include "Utils.hpp"
#include <cstring>

using namespace gc;

Allocator *Allocator::AllocatorObj = nullptr;

Allocator::Allocator(const size_t &size)
    : _size(align(size, 2)) // 16 byte allignment
#ifdef DEBUG
      ,
      _allocated_size(0), _freed_size(0)
#endif // DEBUG
{
    _start = (address)malloc(_size);
    if (_start == nullptr)
    {
        exit_with_error("cannot allocate memory for heap!");
    }
    _end = _start + _size;
    _pos = _start;
}

void Allocator::init(const size_t &size)
{
    switch (GCAlgo)
    {
    case ZEROGC:
        AllocatorObj = new NextFitAllocator(size);
        break;
#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
    case MARKSWEEPGC:
    case THREADED_MC_GC:
    case COMPRESSOR_GC:
        AllocatorObj = new NextFitAllocator(size);
        break;
    case SEMISPACE_COPYING_GC:
        AllocatorObj = new SemispaceNextFitAllocator(size);
        break; // don't have explicit mark phase

#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE
    default:
        fprintf(stderr, "cannot select GC!\n");
        abort();
        break;
    };

#ifdef DEBUG
    if (TraceGCCycles)
    {
        fprintf(stderr, "Chosen GC: %d\n", GCAlgo);
        fprintf(stderr, "Heap: [%p-%p]\n", AllocatorObj->start(), AllocatorObj->end());
    }
#endif // DEBUG
}

void Allocator::release()
{
    delete AllocatorObj;
    AllocatorObj = nullptr;
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
    if (PrintGCStatistics)
    {
        fprintf(stderr, "Allocated bytes: %s\n", printable_size(_allocated_size).c_str());
        fprintf(stderr, "Freed bytes:     %s\n", printable_size(_freed_size).c_str());
    }
}
#endif // DEBUG

ObjectLayout *Allocator::allocate(int tag, size_t size, void *disp_tab)
{
    auto *object = allocate_inner(tag, size, disp_tab);
#ifdef DEBUG
    if (object)
    {
        _allocated_size += size;
    }

    if (PrintAllocatedObjects)
    {
        if (object)
        {
            object->print();
        }
        else
        {
            fprintf(stderr, "Allocation failed!\n");
        }
    }
#endif // DEBUG
    return object;
}

ObjectLayout *Allocator::allocate_inner(int tag, size_t size, void *disp_tab)
{
    if (_pos + size >= _start + _size)
    {
        exit_with_error("cannot allocate memory for object!");
    }

    address object = _pos;
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
    // try to find suitable chunk of the memory
    // compact chunks by the way
    ObjectLayout *chunk = nullptr;
    // assume that we allocate memory consequently between collections
    ObjectLayout *current_chunk = (ObjectLayout *)(_pos ? _pos : _start);

    size_t current_chunk_size = 0;
    while ((address)current_chunk < _end)
    {
        if (current_chunk->_tag != 0)
        {
            // found allocated object

            // merge previous chunks, but it is not ours chunk
            if (current_chunk_size != 0 && chunk != nullptr)
            {
                chunk->_size = current_chunk_size;
            }
            chunk = nullptr;
        }
        else if (current_chunk->_tag == 0 && chunk == nullptr)
        {
            // remember the first free chunk
            if (_pos == nullptr)
            {
                _pos = (address)current_chunk;
            }
            // first free chunk
            chunk = current_chunk;
            current_chunk_size = current_chunk->_size;
        }
        else if (current_chunk->_tag == 0)
        {
            // remember the first free chunk
            if (_pos == nullptr)
            {
                _pos = (address)current_chunk;
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
        current_chunk = (ObjectLayout *)((address)current_chunk + current_chunk->_size);
    }

    if (!chunk || chunk->_size < size)
    {
        return nullptr;
    }

    int appendix_size = 0;

    if (current_chunk_size - size < HEADER_SIZE)
    {
        appendix_size = current_chunk_size - size;
        size = current_chunk_size; // align allocation for correct heap interation
        // this hidden fields always will be zero, so will not affect gc
    }

    if (current_chunk_size - size >= HEADER_SIZE)
    {
        // adjust next chunk
        // at least have memory for the header

        ObjectLayout *next_free = (ObjectLayout *)((address)chunk + size);
        next_free->set_unused(current_chunk_size - size);
        _pos = (address)next_free;
    }

    chunk->_mark = MarkWordUnsetValue;
    chunk->_size = size;
    chunk->_tag = tag;
    chunk->_dispatch_table = disp_tab;

#ifdef DEBUG
    chunk->zero_fields(0xBADBABE);
#endif // DEBUG

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
    if ((address)obj < _pos)
    {
        _pos = (address)obj;
    }
}

void NextFitAllocator::move(const ObjectLayout *src, address dst)
{
    if (dst != (address)src)
    {
#ifdef DEBUG
        if (TraceObjectMoving)
        {
            fprintf(stderr, "Move object [%p-%p] to [%p-%p]\n", src, (address)src + src->_size, dst, dst + src->_size);
        }
#endif // DEBUG

        // non-overlapping memory regions
        if (dst >= (address)src + src->_size || dst <= (address)src - src->_size)
        {
            memcpy(dst, (address)src, src->_size);
        }
        else
        {
            // overlapping memory regions
            memmove(dst, (address)src, src->_size);
        }
    }
}

void NextFitAllocator::force_alloc_pos(address pos)
{
    assert(is_aligned((size_t)pos));

    // create an artificial object with tag 0 and size heap_size
    ObjectLayout *aobj = (ObjectLayout *)pos;
    aobj->set_unused(_end - pos);

    _pos = pos;
}

address NextFitAllocator::next_object(address obj)
{
    ObjectLayout *possible_object = (ObjectLayout *)(obj);

    // search for the first object with non-zero tag
    while ((address)possible_object < _end && possible_object->_tag == 0)
    {
        // assuming size is correct for dead objects
        possible_object = (ObjectLayout *)((address)possible_object + possible_object->_size);
    }

    return (address)possible_object;
}

SemispaceNextFitAllocator::SemispaceNextFitAllocator(const size_t &size) : NextFitAllocator(size)
{
    _orig_heap_start = _start;
    _orig_heap_end = _end;

    _extend = (_end - _start) / 2;
    _end = _start + _extend;

    assert(_start + 2 * _extend == _orig_heap_end);

    force_alloc_pos(_start);

#ifdef DEBUG
    if (TraceGCCycles)
    {
        fprintf(stderr, "Original heap boundaries: [%p-%p], mid = %p\n", _orig_heap_start, _orig_heap_end, _end);
    }
#endif // DEBUG
}

void SemispaceNextFitAllocator::flip()
{
    _start = _end;
    _end = _start + _extend;

    if (_start == _orig_heap_end)
    {
        _start = _orig_heap_start;
        _end = _start + _extend;
    }

#ifdef DEBUG
    if (TraceGCCycles)
    {
        fprintf(stderr, "New heap boundaries: [%p-%p]\n", _start, _end);
    }
#endif // DEBUG

    force_alloc_pos(_start);
}

SemispaceNextFitAllocator::~SemispaceNextFitAllocator()
{
    _start = _orig_heap_start;
    _end = _orig_heap_end;
}
