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
    fprintf(stderr, "Object address = %p\nMark = %x\nTag = %x\nSize = %lu\nDispTable = %p\n", this, _mark, _tag, _size,
            _dispatch_table);

    for (int i = 0; i < field_cnt(); i++)
    {
        fprintf(stderr, "Field %d : %p\n", i, fields_base()[i]);
    }

    fprintf(stderr, "\n");
}
#endif // DEBUG