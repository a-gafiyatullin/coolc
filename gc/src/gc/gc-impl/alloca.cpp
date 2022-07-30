#include "gc/gc-interface/alloca.hpp"
#include "gc/gc-interface/object.hpp"

bool LOGALOC = false;
bool ZEROING = false;

allocator::Alloca::Alloca(size_t size)
    : _size(size)
#ifdef DEBUG
      ,
      _allocated_size(0), _freed_size(0)
#endif // DEBUG
{
    _start = (address)malloc(_size);
    if (_start == NULL)
    {
        throw std::bad_alloc();
    }
    _end = _start + _size;
    _pos = _start;
}

allocator::Alloca::~Alloca()
{
    dump();
    std::free(_start);
}

void allocator::Alloca::dump()
{
#ifdef DEBUG
    std::cout << "Allocated bytes: " << _allocated_size << std::endl;
    std::cout << "Freed bytes: " << _freed_size << std::endl;
#endif // DEBUG
}

address allocator::Alloca::allocate(objects::Klass *klass)
{
    size_t obj_size = klass->size();
    if (_pos + obj_size >= _start + _size)
    {
        dump();
        throw std::bad_alloc(); // for ZeroGC just throw an exception
    }

    address object = _pos;
    _pos += obj_size;

    objects::ObjectHeader *obj_header = (objects::ObjectHeader *)object;
    obj_header->_mark = 0;
    obj_header->_size = obj_size;
    obj_header->_tag = klass->type();

    obj_header->zero_fields();

    LOG_ALLOC(object, obj_header->_size);

    return object;
}

// -------------------------------------------- NextFitAlloca --------------------------------------------
allocator::NextFitAlloca::NextFitAlloca(size_t size) : Alloca(size)
{
    // create an artificial object with tag 0 and size heap_size
    objects::ObjectHeader *aobj = (objects::ObjectHeader *)_start;
    aobj->set_unused(_size);
}

address allocator::NextFitAlloca::allocate(objects::Klass *klass)
{
    size_t obj_size = klass->size();

    // try to find suitable chunk of the memeory
    // compact chunks by the way
    objects::ObjectHeader *chunk = NULL;
    // assume that we allocate memory consequently between collections
    objects::ObjectHeader *current_chunk = (objects::ObjectHeader *)(_pos ? _pos : _start);
    size_t current_chunk_size = 0;
    while ((address)current_chunk < _end)
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
                _pos = (address)current_chunk;
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
                _pos = (address)current_chunk;
            }
            // not the first free contiguous chunk
            // merge previous chunks
            current_chunk_size += current_chunk->_size;
            chunk->_size = current_chunk_size;
        }

        // enough memory?
        if (current_chunk_size >= obj_size)
        {
            break;
        }

        // go to next chunk
        current_chunk = (objects::ObjectHeader *)((address)current_chunk + current_chunk->_size);
    }

    if (!chunk || chunk->_size < obj_size)
    {
        return NULL;
    }

    if (current_chunk_size - obj_size < HEADER_SIZE)
    {
        obj_size = current_chunk_size; // allign allocation for correct heap interation
        // this headen fields always will be zero, so will not affeсt gc
    }

    if (current_chunk_size - obj_size >= HEADER_SIZE)
    {
        // adjust next chunk
        // at least have memory for the header

        objects::ObjectHeader *next_free = (objects::ObjectHeader *)((address)chunk + obj_size);
        next_free->set_unused(current_chunk_size - obj_size);
        _pos = (address)next_free;
    }

    chunk->_mark = 0;
    chunk->_size = obj_size;
    chunk->_tag = klass->type();

    chunk->zero_fields();

    LOG_ALLOC(chunk, chunk->_size);

    return (address)chunk;
}

void allocator::NextFitAlloca::free(address start)
{
    // memory chunk is free if tag of the object starts from this address is 0
    objects::ObjectHeader *hdr = (objects::ObjectHeader *)start;

#ifdef DEBUG
    _freed_size += hdr->_size;
#endif // DEBUG

    hdr->unset_marked();
    if (ZEROING)
    {
        hdr->zero_fields(0xFE);
    }
    hdr->_tag = 0;

    // save size for allocation
    assert(hdr->_size != 0);

    // update the first free pos
    if (start < _pos)
    {
        _pos = start;
    }
}