#pragma once

#include "codegen/constants/Constants.h"
#include <cstdlib>
#include <stdio.h>

extern "C"
{
    extern "C" void *ClassNameTab; // must be defined by coolc. It is the pointer of the first name
    // extern void **ClassObjTab;  // must be defined by coolc

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

    /**
     * @brief Check if two objects are equal
     *
     * @param lo left operand
     * @param ro right operand
     * @return TrueVal for true, FalseVal for false
     */
    int equals(void *lo, void *ro);

    // Basic classes methods
    // ------------------------------ Object ------------------------------
    /**
     * @brief Abort program with debuf info
     *
     * @param receiver Receiver
     * @return NULL
     */
    void *Object_abort(void *receiver);

    /**
     * @brief Get type of the Object
     *
     * @param receiver Receiver
     * @return Cool String
     */
    void *Object_type_name(void *receiver);

    /**
     * @brief Get copy of the Object
     *
     * @param receiver Receiver
     * @return Copy of the Object
     */
    void *Object_copy(void *receiver);

    // ------------------------------ Int ------------------------------
    struct IntLayout
    {
        ObjectLayout _header;
        long long int _value;
    };
    // ------------------------------ String ------------------------------
    struct StringLayout
    {
        ObjectLayout _header;
        IntLayout *_string_size;
        char *_string; // null terminated
    };

    /**
     * @brief Cool String length
     *
     * @param receiver Receiver
     * @return Cool Int for length
     */
    void *String_length(void *receiver);

    /**
     * @brief Cool String concat
     *
     * @param receiver Receiver
     * @param str Other Cool String
     * @return New Cool String
     */
    void *String_concat(void *receiver, void *str);

    /**
     * @brief Cool String substring
     *
     * @param receiver Receiver
     * @param index Starting index
     * @param len Substring length
     * @return New Cool String
     */
    void *String_substr(void *receiver, void *index, void *len);
    // ------------------------------ IO ------------------------------
    /**
     * @brief Print Cool String
     *
     * @param receiver Receiver
     * @param str Cool string for printing
     * @return Receiver
     */
    void *IO_out_string(void *receiver, void *str);

    /**
     * @brief Print Cool Int
     *
     * @param receiver Receiver
     * @param integer Cool Int for printing
     * @return Receiver
     */
    void *IO_out_int(void *receiver, void *integer);

    /**
     * @brief Read line to Cool String
     *
     * @param receiver Receiver
     * @return New Cool String
     */
    void *IO_in_string(void *receiver);

    /**
     * @brief Read line to Cool Int
     *
     * @param receiver Receiver
     * @return New Cool Int
     */
    void *IO_in_int(void *receiver);

    // -------------------------------------- GC --------------------------------------

    /**
     * @brief Allocate object using tag
     *
     * @param tag Class tag
     * @return Pointer to the newly allocated object
     */
    void *gc_alloc_by_tag(int tag);

    /**
     * @brief Allocate object with known size
     *
     * @param tag Object tag
     * @param size Object size
     * @param disp_tab Dispatch table ptr
     * @return Pointer to the newly allocated object
     */
    void *gc_alloc(int tag, size_t size, void *disp_tab);
}