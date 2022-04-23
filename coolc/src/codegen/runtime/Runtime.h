#pragma once

#include "codegen/constants/Constants.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>

extern "C"
{
    extern "C" void *ClassNameTab; // must be defined by coolc. It is the pointer of the first name
    extern "C" int IntTag;
    extern "C" int BoolTag;
    extern "C" int StringTag;

    /**
     * @brief Structure of the Object header
     *
     */
    struct ObjectLayout
    {
        MARK_TYPE _mark;
        TAG_TYPE _tag;
        SIZE_TYPE _size;
        DISP_TAB_TYPE _dispatch_table;
    };

    struct IntLayout
    {
        ObjectLayout _header;
        long long int _value;
    };

    struct BoolLayout
    {
        ObjectLayout _header;
        long long int _value;
    };

    struct StringLayout
    {
        ObjectLayout _header;
        IntLayout *_string_size;
        char *_string; // null terminated
    };

    /**
     * @brief Check if two objects are equal
     *
     * @param lo left operand
     * @param ro right operand
     * @return TrueVal for true, FalseVal for false
     */
    int equals(ObjectLayout *lo, ObjectLayout *ro);

    /**
     * @brief Abort on case mismatch
     *
     * @param tag Class tag
     */
    void case_abort(int tag);

    // Basic classes methods
    // ------------------------------ Object ------------------------------
    /**
     * @brief Abort program with debuf info
     *
     * @param receiver Receiver
     * @return NULL
     */
    ObjectLayout *Object_abort(ObjectLayout *receiver);

    /**
     * @brief Get type of the Object
     *
     * @param receiver Receiver
     * @return Cool String
     */
    StringLayout *Object_type_name(ObjectLayout *receiver);

    /**
     * @brief Get copy of the Object
     *
     * @param receiver Receiver
     * @return Copy of the Object
     */
    ObjectLayout *Object_copy(ObjectLayout *receiver);

    // ------------------------------ String ------------------------------

    /**
     * @brief Cool String length
     *
     * @param receiver Receiver
     * @return Cool Int for length
     */
    IntLayout *String_length(StringLayout *receiver);

    /**
     * @brief Cool String concat
     *
     * @param receiver Receiver
     * @param str Other Cool String
     * @return New Cool String
     */
    StringLayout *String_concat(StringLayout *receiver, StringLayout *str);

    /**
     * @brief Cool String substring
     *
     * @param receiver Receiver
     * @param index Starting index
     * @param len Substring length
     * @return New Cool String
     */
    StringLayout *String_substr(StringLayout *receiver, IntLayout *index, IntLayout *len);
    // ------------------------------ IO ------------------------------
    /**
     * @brief Print Cool String
     *
     * @param receiver Receiver
     * @param str Cool string for printing
     * @return Receiver
     */
    ObjectLayout *IO_out_string(ObjectLayout *receiver, StringLayout *str);

    /**
     * @brief Print Cool Int
     *
     * @param receiver Receiver
     * @param integer Cool Int for printing
     * @return Receiver
     */
    ObjectLayout *IO_out_int(ObjectLayout *receiver, IntLayout *integer);

    /**
     * @brief Read line to Cool String
     *
     * @param receiver Receiver
     * @return New Cool String
     */
    StringLayout *IO_in_string(ObjectLayout *receiver);

    /**
     * @brief Read line to Cool Int
     *
     * @param receiver Receiver
     * @return New Cool Int
     */
    IntLayout *IO_in_int(ObjectLayout *receiver);

    // -------------------------------------- GC --------------------------------------

    /**
     * @brief Allocate object with known size
     *
     * @param tag Object tag
     * @param size Object size
     * @param disp_tab Dispatch table ptr
     * @return Pointer to the newly allocated object
     */
    ObjectLayout *gc_alloc(int tag, size_t size, void *disp_tab);
}