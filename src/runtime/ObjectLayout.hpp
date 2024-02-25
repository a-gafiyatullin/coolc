#pragma once

#include "codegen/constants/Constants.h"
#include "globals.hpp"

#define FIELD_SIZE sizeof(address)

extern "C"
{
    extern "C" void *class_nameTab; // NOLINT // must be defined by coolc. It is the pointer of the first name
    extern "C" int _int_tag;        // NOLINT
    extern "C" int _bool_tag;       // NOLINT
    extern "C" int _string_tag;     // NOLINT

    extern "C" void *String_dispTab; // NOLINT // because we need it in IO
    extern "C" void *Int_dispTab;    // NOLINT // because we need it in IO
};

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

    /**
     * @brief Zero hidden fields
     *
     * @param appendix_size Size of hidden fields
     */
    void zero_appendix(int appendix_size);

    /**
     * @brief Check object mark word
     *
     * @return true it is marked
     * @return false it is not marked
     */
    inline bool is_marked() const { return _mark != MarkWordUnsetValue; }

    /**
     * @brief Set mark word of the object
     *
     */
    inline void set_marked() { _mark = MarkWordSetValue; }

    /**
     * @brief Unset mark word of the object
     *
     * @param obj Object header to unset
     */
    inline void unset_marked() { _mark = MarkWordUnsetValue; }

    /**
     * @brief Set object as unused
     *
     * @param size Size of the object
     */
    inline void set_unused(size_t size)
    {
        _size = size;
        _mark = MarkWordUnsetValue;
        _tag = UnusedTag;
    }

    /**
     * @brief Get the number of the fields
     *
     * @return int Number of the fields
     */
    inline int field_cnt() const
    {
        static const int HEADER_SIZE = sizeof(ObjectLayout);
        return (_size - HEADER_SIZE) / FIELD_SIZE;
    }

    /**
     * @brief Get the object fields base address
     *
     * @return address * Fields base address
     */
    inline address *fields_base() const
    {
        static const int HEADER_SIZE = sizeof(ObjectLayout);
        return (address *)((address)this + HEADER_SIZE);
    }

    /**
     * @brief Check if the object has special type
     *
     * @return true it is of the special type
     * @return false it is not of the special type
     */
    inline bool has_special_type() const { return _tag == _int_tag || _tag == _bool_tag || _tag == _string_tag; }

    /**
     * @brief Check if object is string
     *
     * @return true if it is a string
     * @return false if it isn't a string
     */
    inline bool is_string() const { return _tag == _string_tag; }

    /**
     * @brief Fill fields with zero
     *
     * @param val Zeroing value
     */
    void zero_fields(int val = 0);

#ifdef DEBUG
    /**
     * @brief Print object
     *
     */
    void print();
#endif // DEBUG
};

struct IntLayout : public ObjectLayout
{
    long long int _value;
};

struct BoolLayout : public ObjectLayout
{
    long long int _value;
};

struct StringLayout : public ObjectLayout
{
    IntLayout *_string_size;
    char _string[1]; // null terminated
};
