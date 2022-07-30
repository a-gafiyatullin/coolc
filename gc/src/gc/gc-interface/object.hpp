#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#ifdef DEBUG
#include <iostream>
#endif // DEBUG

using address = char *;
using address_fld = address *;

using byte = uint8_t;
using halfword = uint16_t;
using word = uint32_t;
using doubleword = uint64_t;

// will see it further
#define INTOBJ objects::Klass::Integer
#define LLNODE objects::Klass::LinkedListNode

#define FIELD_SIZE sizeof(address)
#define HEADER_SIZE sizeof(objects::ObjectHeader)

namespace objects
{

// tags variants
enum ObjectType
{
    INTEGER = 1,
    OTHER
};

/**
 * @brief ObjectHeader describes object
 *
 */
struct ObjectHeader
{
    int _mark;
    int _tag;
    size_t _size;

  public:
    /**
     * @brief Check object mark word
     *
     * @return true it is marked
     * @return false it is not marked
     */
    inline bool is_marked() const
    {
        return _mark != 0;
    }

    /**
     * @brief Set mark word of the object
     *
     */
    inline void set_marked()
    {
        _mark = 1;
    }

    /**
     * @brief Unset mark word of the object
     *
     * @param obj Object header to unset
     */
    inline void unset_marked()
    {
        _mark = 0;
    }

    /**
     * @brief Set object as unsed
     *
     * @param size Size of the object
     */
    inline void set_unused(size_t size)
    {
        _size = size;
        _mark = 0;
        _tag = 0;
    }

    /**
     * @brief Get the number of the fields
     *
     * @return int Number of the fields
     */
    inline int field_cnt() const
    {
        return (_size - HEADER_SIZE) / FIELD_SIZE;
    }

    /**
     * @brief Get the object fields base address
     *
     * @return address_fld Fields base address
     */
    inline address_fld fields_base() const
    {
        return (address_fld)((address)this + HEADER_SIZE);
    }

    /**
     * @brief Check if the object has special type
     *
     * @return true it is of the special type
     * @return false it is not of the special type
     */
    inline bool has_special_type() const
    {
        return _tag < OTHER;
    }

    /**
     * @brief Fill fields with zero
     *
     * @param val Zeroing value
     */
    void zero_fields(int val = 0);

#ifdef DEBUG
    /**
     * @brief Print header
     *
     */
    void print();
#endif // DEBUG
};

/**
 * @brief Klass describes types of the objects
 *
 */
class Klass
{
  private:
    const int _fields_count;
    const ObjectType _type;

  public:
    // some common prebuild types
    static Klass Integer;
    static Klass LinkedListNode;

    /**
     * @brief Construct a Klass
     *
     * @param fields_count Number of the pointer-sized fields
     * @param type Object Type
     */
    explicit Klass(int fields_count, ObjectType type) : _fields_count(fields_count), _type(type)
    {
    }

    /**
     * @brief Get offset to the field
     *
     * @param field_num Number of the field
     * @return offset to the field
     */
    __attribute__((always_inline)) size_t offset(int field_num) const
    {
        assert(field_num < _fields_count);
        return HEADER_SIZE + field_num * FIELD_SIZE; // all fields are pointers or pointer-sized for now
    }

    /**
     * @brief Size of the object of this type
     *
     * @return size_t Size
     */
    inline size_t size() const
    {
        return HEADER_SIZE + _fields_count * FIELD_SIZE;
    }

    /**
     * @brief Type of the object
     *
     * @return ObjectType
     */
    inline ObjectType type() const
    {
        return _type;
    }
};

}; // namespace objects