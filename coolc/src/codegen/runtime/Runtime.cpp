#include "Runtime.h"

void *Object_abort(void *receiver)
{
    int tag = ((ObjectLayout *)receiver)->_tag;
    StringLayout *name = (StringLayout *)(((void **)&ClassNameTab)[tag]);

    fprintf(stderr, "Abort called from class %s", (char *)name->_string);

    exit(-1);

    return nullptr;
}

void *gc_alloc(int tag, size_t size, void *disp_tab)
{
    // TODO: dummy allocation. Should be managed by GC
    void *object = malloc(size);

    ObjectLayout *layout = (ObjectLayout *)object;
    layout->_mark = MarkWordDefaultValue;
    layout->_tag = tag;
    layout->_size = size;
    layout->_dispatch_table = disp_tab;

    return object;
}

void *IO_out_string(void *receiver, void *str)
{
    StringLayout *layout = (StringLayout *)str;

    printf("%s", layout->_string);

    return receiver;
}

void *Object_type_name(void *receiver)
{
}

void *Object_copy(void *receiver)
{
}

void *String_length(void *receiver)
{
}

void *String_concat(void *receiver, void *str)
{
}

void *String_substr(void *receiver, void *index, void *len)
{
}

void *IO_in_int(void *receiver)
{
}

void *IO_in_string(void *receiver)
{
}

void *IO_out_int(void *receiver, void *integer)
{
}