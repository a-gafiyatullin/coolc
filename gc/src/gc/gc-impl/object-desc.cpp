#include "gc/gc-interface/object-desc.hpp"

objects::Klass INTOBJ(1, INTEGER);
objects::Klass LLNODE(2, OTHER);

void objects::ObjectHeader::zero_fields(int val)
{
    memset(fields_base(), val, _size - HEADER_SIZE);
}

#ifdef DEBUG
void objects::ObjectHeader::print()
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