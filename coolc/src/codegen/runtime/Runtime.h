#pragma once

#include "codegen/constants/Constants.h"
#include <cstdlib>
#include <stdio.h>

extern "C"
{
    //extern void **ClassNameTab; // must be defined by coolc
    //extern void **ClassObjTab;  // must be defined by coolc

    /**
     * @brief Structure of the Object header
     *
     */
    struct ObjectLayout
    {
        long int _mark;
        long int _tag;
        long int _size;
        void *_dispatch_table;
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

    /**
     * @brief Initialize Object object
     *
     * @param receiver Receiver
     * @param dispatch_table Pointer to dispatch table
     * @param tag Object tag
     */
    void Object_init(void *receiver, void *dispatch_table, int tag);
    // ------------------------------ Int ------------------------------
    struct IntLayout
    {
        ObjectLayout _header;
        long int _value;
    };

    /**
     * @brief Initialize Int object
     *
     * @param receiver Receiver
     * @param dispatch_table Pointer to dispatch table
     * @param tag Int tag
     */
    void Int_init(void *receiver, void *dispatch_table, int tag);
    // ------------------------------ String ------------------------------
    struct StringLayout
    {
        ObjectLayout _header;
        void *_string_size;
        void *_string; // null terminated
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
     * @param size Object size
     * @return Pointer to the newly allocated object
     */
    void *gc_alloc(size_t size);
}