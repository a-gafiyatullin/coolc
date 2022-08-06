#pragma once

#include "gc/gc-interface/object.hpp"

template <class ObjectHeaderType>
objects::Klass<ObjectHeaderType> objects::Klass<ObjectHeaderType>::Integer(1, INTEGER);

template <class ObjectHeaderType>
objects::Klass<ObjectHeaderType> objects::Klass<ObjectHeaderType>::LinkedListNode(2, OTHER);

template <class MarkWordType, class TagWordType, class SizeWordType>
void objects::ObjectHeader<MarkWordType, TagWordType, SizeWordType>::zero_fields(int val)
{
    memset(fields_base(), val, _size - sizeof(ObjectHeader));
}

#ifdef DEBUG
template <class MarkWordType, class TagWordType, class SizeWordType>
void objects::ObjectHeader<MarkWordType, TagWordType, SizeWordType>::print()
{
    std::cerr << "Address: " << std::hex << (uint64_t)this << std::endl;
    std::cerr << "Mark: " << _mark << std::endl;
    std::cerr << "Tag: " << _tag << std::endl;
    std::cerr << "Size: " << _size << std::endl;

    for (int i = 0; i < field_cnt(); i++)
    {
        std::cerr << "Field " << i << ": " << std::hex << (uint64_t)fields_base()[i] << std::endl;
    }
}
#endif // DEBUG