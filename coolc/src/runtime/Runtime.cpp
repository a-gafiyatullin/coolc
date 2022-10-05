#include "Runtime.h"
#include "gc/GC.hpp"
#include "gc/Utils.hpp"
#include "globals.hpp"
#include <cstring>

void _init_runtime(int argc, char **argv) // NOLINT
{
    process_runtime_args(argc, argv);

    gc::Allocator::init(std::max(str_to_size(MaxHeapSize), sizeof(ObjectLayout)));
    gc::StackWalker::init();
    gc::Marker::init();
    gc::GC::init((gc::GC::GcType)GCAlgo);
}

void _finish_runtime() // NOLINT
{
    gc::GC::release();
    gc::Marker::release();
    gc::StackWalker::release();
    gc::Allocator::release();
}

ObjectLayout *Object_abort(ObjectLayout *receiver) // NOLINT
{
    auto *const name =
        reinterpret_cast<StringLayout *>(((void **)&class_nameTab)[receiver->_tag - 1]); // because tag 0 is reserved

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
    return reinterpret_cast<StringLayout *>(((void **)&class_nameTab)[receiver->_tag - 1]); // because tag 0 is reserved
}

ObjectLayout *Object_copy(ObjectLayout *receiver) // NOLINT
{
    gc::GC::gc()->add_runtime_root((address *)&receiver);

    auto *const copy = gc::GC::gc()->copy(receiver);

    gc::GC::gc()->clean_runtime_roots();

    return copy;
}

IntLayout *String_length(StringLayout *receiver) // NOLINT
{
    return receiver->_string_size;
}

IntLayout *make_int(const int &value, void *int_disp_tab)
{
    auto *const int_obj = reinterpret_cast<IntLayout *>(_gc_alloc(_int_tag, sizeof(IntLayout), int_disp_tab));
    int_obj->_value = value;

    return int_obj;
}

StringLayout *String_concat(StringLayout *receiver, StringLayout *str) // NOLINT
{
    int size = str->_string_size->_value + receiver->_string_size->_value + sizeof(StringLayout);

    gc::GC::gc()->add_runtime_root((address *)&receiver);
    gc::GC::gc()->add_runtime_root((address *)&str);

    auto *new_string = (StringLayout *)_gc_alloc(_string_tag, size, str->_dispatch_table);

    gc::GC::gc()->add_runtime_root((address *)&new_string); // preserve string before integer allocation
    new_string->_string_size = NULL;                        // need correct address for GC

    // set size
    auto *const new_int =
        make_int(str->_string_size->_value + receiver->_string_size->_value, receiver->_string_size->_dispatch_table);

    new_string->_string_size = new_int;

    // copy strings
    memcpy(new_string->_string, receiver->_string, receiver->_string_size->_value);
    memcpy(new_string->_string + receiver->_string_size->_value, str->_string, str->_string_size->_value);
    new_string->_string[new_string->_string_size->_value] = '\0';

    gc::GC::gc()->clean_runtime_roots();

    return new_string;
}

StringLayout *String_substr(StringLayout *receiver, IntLayout *index, IntLayout *len) // NOLINT
{
    int index_val = index->_value;

    gc::GC::gc()->add_runtime_root((address *)&len);
    gc::GC::gc()->add_runtime_root((address *)&receiver);

    auto *const new_string =
        (StringLayout *)_gc_alloc(_string_tag, len->_value + sizeof(StringLayout), receiver->_dispatch_table);

    new_string->_string_size = len;

    memcpy(new_string->_string, receiver->_string + index_val, len->_value);
    new_string->_string[new_string->_string_size->_value] = '\0';

    gc::GC::gc()->clean_runtime_roots();

    return new_string;
}

IntLayout *IO_in_int(ObjectLayout *receiver) // NOLINT
{
    // TODO: any problems here?
    long long int value;

    scanf("%lld", &value);

    return make_int(value, Int_dispTab);
}

StringLayout *IO_in_string(ObjectLayout *receiver) // NOLINT
{
    // TODO: any problems here?
    char str[1025];

    scanf("%s", str);
    int len = strlen(str);

    StringLayout *obj = (StringLayout *)_gc_alloc(_string_tag, sizeof(StringLayout) + len, &String_dispTab);
    gc::GC::gc()->add_runtime_root((address *)&obj); // preserve string before integer allocation
    obj->_string_size = NULL;                        // need correct address for GC

    auto *const new_int = make_int(len, &Int_dispTab);

    obj->_string_size = new_int;

    memcpy(obj->_string, str, len);
    obj->_string[len] = '\0';

    gc::GC::gc()->clean_runtime_roots();

    return obj;
}

ObjectLayout *IO_out_int(ObjectLayout *receiver, IntLayout *integer) // NOLINT
{
    assert(integer->_tag == _int_tag);
    printf("%lld", integer->_value);

    return receiver;
}

void _case_abort(int tag) // NOLINT
{
    auto *const name =
        reinterpret_cast<StringLayout *>(((void **)&class_nameTab)[tag - 1]); // because tag 0 is reserved
    printf("No match in case statement for Class %s", name->_string);

    exit(-1);
}

int _equals(ObjectLayout *lo, ObjectLayout *ro) // NOLINT
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

    if (lo_tag == _bool_tag)
    {
        return reinterpret_cast<BoolLayout *>(lo)->_value == reinterpret_cast<BoolLayout *>(ro)->_value ? TrueValue
                                                                                                        : FalseValue;
    }

    if (lo_tag == _int_tag)
    {
        return reinterpret_cast<IntLayout *>(lo)->_value == reinterpret_cast<IntLayout *>(ro)->_value ? TrueValue
                                                                                                      : FalseValue;
    }

    if (lo_tag == _string_tag)
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

void _dispatch_abort(StringLayout *filename, int linenumber) // NOLINT
{
    printf("%s:%d: Dispatch to void.", filename->_string, linenumber);

    exit(-1);
}

void _case_abort_2(StringLayout *filename, int linenumber) // NOLINT
{
    printf("%s:%dMatch on void in case statement.", filename->_string, linenumber);

    exit(-1);
}

ObjectLayout *_gc_alloc(int tag, size_t size, void *disp_tab) // NOLINT
{
    return gc::GC::gc()->allocate(tag, size, disp_tab);
}