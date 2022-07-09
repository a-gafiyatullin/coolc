#include "gc/gc-interface/object-desc.hpp"

objects::Klass INTOBJ(1, INTEGER);
objects::Klass LLNODE(2, OTHER);

void objects::ObjectHeader::zero_fields()
{
    memset(fields_base(), 0, _size - HEADER_SIZE);
}