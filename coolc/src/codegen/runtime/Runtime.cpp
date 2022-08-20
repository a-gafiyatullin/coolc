#include "Runtime.h"
#include "gc/GC.hpp"

void init_runtime(int argc, char **argv)
{
    gc::GC::init(gc::GC::ZEROGC, 384 * 1024);
}

ObjectLayout *Object_abort(ObjectLayout *receiver) // NOLINT
{
    auto *const name =
        reinterpret_cast<StringLayout *>(((void **)&ClassNameTab)[receiver->_tag - 1]); // because tag 0 is reserved

    printf("Abort called from class %s", name->_string);

    exit(-1);

    return nullptr;
}

ObjectLayout *IO_out_string(ObjectLayout *receiver, StringLayout *str) // NOLINT
{
    printf("%s", str->_string);

    return receiver;
}

StringLayout *Object_type_name(ObjectLayout *receiver) // NOLINT
{
    return reinterpret_cast<StringLayout *>(((void **)&ClassNameTab)[receiver->_tag - 1]); // because tag 0 is reserved
}

ObjectLayout *Object_copy(ObjectLayout *receiver) // NOLINT
{
    return gc::GC::gc()->copy(receiver);
}

IntLayout *String_length(StringLayout *receiver) // NOLINT
{
    return receiver->_string_size;
}

IntLayout *make_int(const int &value, void *int_disp_tab)
{
    auto *const int_obj = reinterpret_cast<IntLayout *>(gc_alloc(IntTag, sizeof(IntLayout), int_disp_tab));
    int_obj->_value = value;

    return int_obj;
}

StringLayout *String_concat(StringLayout *receiver, StringLayout *str) // NOLINT
{
    auto *const new_string = reinterpret_cast<StringLayout *>(Object_copy(reinterpret_cast<ObjectLayout *>(receiver)));

    new_string->_string_size =
        make_int(str->_string_size->_value + receiver->_string_size->_value, receiver->_string_size->_dispatch_table);
    new_string->_string = reinterpret_cast<char *>(malloc(new_string->_string_size->_value + 1));
    memcpy(new_string->_string, receiver->_string, receiver->_string_size->_value);
    memcpy(new_string->_string + receiver->_string_size->_value, str->_string, str->_string_size->_value);
    new_string->_string[new_string->_string_size->_value] = '\0';

    return new_string;
}

StringLayout *String_substr(StringLayout *receiver, IntLayout *index, IntLayout *len) // NOLINT
{
    auto *const new_string = reinterpret_cast<StringLayout *>(Object_copy(reinterpret_cast<ObjectLayout *>(receiver)));

    new_string->_string_size = len;
    new_string->_string = reinterpret_cast<char *>(malloc(len->_value + 1));
    memcpy(new_string->_string, receiver->_string + index->_value, len->_value);
    new_string->_string[new_string->_string_size->_value] = '\0';

    return new_string;
}

IntLayout *IO_in_int(ObjectLayout *receiver) // NOLINT
{
}

StringLayout *IO_in_string(ObjectLayout *receiver) // NOLINT
{
}

ObjectLayout *IO_out_int(ObjectLayout *receiver, IntLayout *integer) // NOLINT
{
    assert(integer->_tag == IntTag);
    printf("%lld", integer->_value);

    return receiver;
}

void case_abort(int tag)
{
    auto *const name = reinterpret_cast<StringLayout *>(((void **)&ClassNameTab)[tag - 1]); // because tag 0 is reserved
    printf("No match in case statement for Class %s", name->_string);

    exit(-1);
}

int equals(ObjectLayout *lo, ObjectLayout *ro)
{
    if (!lo || !ro)
    {
        return FalseValue;
    }

    const auto &lo_tag = lo->_tag;
    const auto &ro_tag = ro->_tag;

    if (lo_tag != ro_tag)
    {
        return FalseValue;
    }

    if (lo_tag == BoolTag)
    {
        return reinterpret_cast<BoolLayout *>(lo)->_value == reinterpret_cast<BoolLayout *>(ro)->_value ? TrueValue
                                                                                                        : FalseValue;
    }

    if (lo_tag == IntTag)
    {
        return reinterpret_cast<IntLayout *>(lo)->_value == reinterpret_cast<IntLayout *>(ro)->_value ? TrueValue
                                                                                                      : FalseValue;
    }

    if (lo_tag == StringTag)
    {
        auto *const str1 = reinterpret_cast<StringLayout *>(lo);
        auto *const str2 = reinterpret_cast<StringLayout *>(ro);

        if (str1->_string_size->_value != str2->_string_size->_value)
        {
            return FalseValue;
        }

        return !strcmp(str1->_string, str2->_string) ? TrueValue : FalseValue;
    }

    return FalseValue;
}

void dispatch_abort(StringLayout *filename, int linenumber)
{
    printf("%s:%d: Dispatch to void.", filename->_string, linenumber);

    exit(-1);
}

void case_abort_2(StringLayout *filename, int linenumber)
{
    printf("%s:%dMatch on void in case statement.", filename->_string, linenumber);

    exit(-1);
}

ObjectLayout *gc_alloc(int tag, size_t size, void *disp_tab) // NOLINT
{
    return gc::GC::gc()->allocate(tag, size, disp_tab);
}