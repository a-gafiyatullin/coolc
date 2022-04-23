#include "Runtime.h"

ObjectLayout *Object_abort(ObjectLayout *receiver)
{
    StringLayout *name = (StringLayout *)(((void **)&ClassNameTab)[receiver->_tag]);

    fprintf(stderr, "Abort called from class %s", (char *)name->_string);

    exit(-1);

    return nullptr;
}

ObjectLayout *gc_alloc(int tag, size_t size, void *disp_tab)
{
    // TODO: dummy allocation. Should be managed by GC
    void *object = malloc(size);

    ObjectLayout *layout = (ObjectLayout *)object;
    layout->_mark = MarkWordDefaultValue;
    layout->_tag = tag;
    layout->_size = size;
    layout->_dispatch_table = disp_tab;

    return layout;
}

ObjectLayout *IO_out_string(ObjectLayout *receiver, StringLayout *str)
{
    printf("%s", str->_string);

    return receiver;
}

StringLayout *Object_type_name(ObjectLayout *receiver)
{
    return (StringLayout *)(((void **)&ClassNameTab)[receiver->_tag]);
}

ObjectLayout *Object_copy(ObjectLayout *receiver)
{
    // TODO: dummy allocation. Should be managed by GC
    ObjectLayout *object = (ObjectLayout *)malloc(receiver->_size);
    memcpy(object, receiver, receiver->_size);

    return object;
}

IntLayout *String_length(StringLayout *receiver)
{
    return receiver->_string_size;
}

StringLayout *String_concat(StringLayout *receiver, StringLayout *str)
{
}

StringLayout *String_substr(StringLayout *receiver, IntLayout *index, IntLayout *len)
{
}

IntLayout *IO_in_int(ObjectLayout *receiver)
{
}

StringLayout *IO_in_string(ObjectLayout *receiver)
{
}

ObjectLayout *IO_out_int(ObjectLayout *receiver, IntLayout *integer)
{
    printf("%lld", integer->_value);

    return receiver;
}

void case_abort(int tag)
{
    StringLayout *name = (StringLayout *)(((void **)&ClassNameTab)[tag]);
    fprintf(stderr, "No match in case statement for Class %s", (char *)name->_string);

    exit(-1);
}