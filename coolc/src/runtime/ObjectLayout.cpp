#include "ObjectLayout.hpp"
#include <cstdio>
#include <cstring>

void ObjectLayout::zero_fields(int val)
{
    static const int HEADER_SIZE = sizeof(ObjectLayout);
    memset(fields_base(), val, _size - HEADER_SIZE);
}

void ObjectLayout::zero_appendix(int appendix_size)
{
    memset((address)this + _size - appendix_size, 0, appendix_size);
}

#ifdef DEBUG
void ObjectLayout::print()
{
    fprintf(stderr, "%p: Mark = %x; Tag = %x; Size = %lu; DispTable = %p\n", this, _mark, _tag, _size, _dispatch_table);
}
#endif // DEBUG