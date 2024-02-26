#include "Allocator.hpp"
#include <cassert>

allocator::LinearAllocator *myir::IRObject::Alloca = nullptr;

allocator::LinearAllocator::LinearAllocator(int chunk_size, bool is_global) : _chunk_size(chunk_size)
{
    grow();

    if (is_global)
    {
        myir::IRObject::set_alloca(this);
    }
}

bool allocator::LinearAllocator::operator==(const LinearAllocator &rhs) const { return &_chunks == &rhs._chunks; }

allocator::LinearAllocator::~LinearAllocator()
{
    for (auto ch : _chunks)
    {
        if (ch._start)
        {
            delete[] ch._start;
            ch._start = nullptr;
        }
    }
}

void allocator::LinearAllocator::grow()
{
    auto *start = new char[_chunk_size];
    _chunks.push_back({start, start});
}

void *allocator::LinearAllocator::allocate(size_t size, size_t align)
{
    assert(size);

    auto *curr_start = _chunks.back()._start;
    auto *curr_pos = _chunks.back()._pos;

    int addition = 0;
    if ((size_t)curr_pos % align)
        addition = (size_t)curr_pos - ((size_t)curr_pos % align);

    // size reqs
    if ((curr_start + _chunk_size) - curr_pos < size + addition)
    {
        grow();
        return allocate(size, align);
    }

    // align reqs
    if ((size_t)curr_pos % align)
    {
        curr_pos += addition;
    }

    _chunks.back()._pos = curr_pos + size;
    return curr_pos;
}
