#include "Runtime.h"

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

void *gc_alloc(int tag, size_t size, void *disp_tab)
{
    // TODO: dummy
    void *object = calloc(1, size);

    ObjectLayout *layout = (ObjectLayout *)object;
    layout->_mark = 0;
    layout->_tag = tag;
    layout->_size = size;
    layout->_dispatch_table = disp_tab;

    return object;
}