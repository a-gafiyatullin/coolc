#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

using address = char *;

using byte = uint8_t;
using halfword = uint16_t;
using word = uint32_t;
using doubleword = uint64_t;

namespace objects
{

struct ObjectHeader
{
    int _mark;
    int _tag; // unused for now
    size_t _size;
};

class Klass
{
  private:
    const int _fields_count;

  public:
    explicit Klass(const int &fields_count) : _fields_count(fields_count)
    {
    }

    __attribute__((always_inline)) size_t offset(const int &field_num) const
    {
        assert(field_num < _fields_count);
        return sizeof(ObjectHeader) + field_num * sizeof(address); // all fields are pointers or pointer-sized for now
    }

    inline size_t size() const
    {
        return sizeof(ObjectHeader) + _fields_count * sizeof(address);
    }
};

}; // namespace objects