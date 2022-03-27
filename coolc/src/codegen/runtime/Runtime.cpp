#include "Runtime.h"

void Object_init(void *receiver, void *dispatch_table, int tag)
{
    ObjectLayout *object = (ObjectLayout *)receiver;

    object->_mark = 0;
    object->_tag = tag;
    object->_dispatch_table = dispatch_table;
    object->_size = sizeof(ObjectLayout);
}

void Int_init(void *receiver, void *dispatch_table, int tag)
{
    Object_init(receiver, dispatch_table, tag);

    IntLayout *integer = (IntLayout *)receiver;

    integer->_header._size = sizeof(IntLayout);
    integer->_value = 0;
}

void *Object_abort(void *receiver)
{
    /*int tag = ((ObjectLayout *)receiver)->_tag;
    StringLayout *name = (StringLayout *)(ClassNameTab + tag);

    fprintf(stderr, "Abort called from class %s", (char *)name->_string);
    exit(-1);

    return nullptr;*/
}

void *gc_alloc_by_tag(int tag)
{
    // TODO: dummy
}

void *gc_alloc(size_t size)
{
    // TODO: dummy
    return calloc(1, size);
}