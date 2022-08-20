#include "ObjectLayout.hpp"

void ObjectLayout::zero_fields(int val)
{
    static const int HEADER_SIZE = sizeof(ObjectLayout);
    memset(fields_base(), val, _size - HEADER_SIZE);
}

void ObjectLayout::zero_appendix(int appendix_size)
{
    memset((gc_address)this + _size - appendix_size, 0, _size - appendix_size);
}

#ifdef DEBUG
void ObjectLayout::print()
{
    fprintf(stderr, "Address = %lx\nMark = %x\nTag = %x\nSize = %lu\nDispTable = %lx\n", (uint64_t)this, _mark, _tag,
            _size, (uint64_t)_dispatch_table);

    for (int i = 0; i < field_cnt(); i++)
    {
        fprintf(stderr, "Field %d : %lx\n", i, (uint64_t)(fields_base()[i]));
    }

    fprintf(stderr, "\n");
}
#endif // DEBUG