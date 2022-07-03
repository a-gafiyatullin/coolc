#pragma once

#include "object-desc.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

namespace gc
{

#define guarantee(cond, msg)                                                                                           \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        std::cerr << "Condition " #cond " failed: " msg << std::endl;                                                  \
        abort();                                                                                                       \
    }

#define UNIMPEMENTED(method)                                                                                           \
    std::cerr << "Unimplemented method: " method << std::endl;                                                         \
    abort();

class StackRecord;

/**
 * @brief Base class for all GCs
 *
 */
class GC
{
    friend class StackRecord;

  protected:
    StackRecord *_current_scope; // emulate stack

  public:
    /**
     * @brief Allocate object of the klass
     *
     * @param klass Klass handle
     * @return address to the start of the object
     */
    address allocate(objects::Klass *klass)
    {
        UNIMPEMENTED("allocate");
    }

    /**
     * @brief Write to memory
     *
     * @tparam T type of operation
     * @param base Base address
     * @param offset Offset
     * @param src Data to write
     */
    template <class T> __attribute__((always_inline)) void write(address base, std::size_t offset, T src)
    {
        UNIMPEMENTED("write");
    }

    /**
     * @brief Read memory
     *
     * @tparam T type of operation
     * @param base Base address
     * @param offset Offset
     * @return Data
     */
    template <class T> __attribute__((always_inline)) T read(address base, std::size_t offset)
    {
        UNIMPEMENTED("read");
    }

    /**
     * @brief Collect garbage
     *
     */
    void collect()
    {
        UNIMPEMENTED("collect");
    }
};

class ZeroGC : public GC
{
  protected:
    const size_t _heap_size;

    address _heap_start;
    address _heap_pos;

  public:
    ZeroGC(const size_t &heap_size);

    address allocate(objects::Klass *klass);

    template <class T> __attribute__((always_inline)) void write(address base, std::size_t offset, T src)
    {
        *((T *)(base + offset)) = src;
    }

    template <class T> __attribute__((always_inline)) T read(address base, std::size_t offset)
    {
        return *((T *)(base + offset));
    }
};

/**
 * @brief StackRecord tracks root objects
 *
 */
class StackRecord
{
  private:
    StackRecord *_parent;

    std::vector<address> _objects;

    GC *_gc;

  public:
    /**
     * @brief Construct a new StackRecord and adjust GC state
     *
     * @param gc Assosiated GC
     */
    StackRecord(GC *gc);

    /**
     * @brief Construct a new StackRecord and adjust GC state
     *
     * @param gc Assosiated GC
     * @param parent Parent scope
     */
    StackRecord(GC *gc, StackRecord *parent);

    /**
     * @brief Destroy the StackRecord and adjust GC state
     *
     */
    ~StackRecord();

    /**
     * @brief Add a new root
     *
     */
    __attribute__((always_inline)) void reg_root(address obj)
    {
        _objects.push_back(obj);
    }
};

}; // namespace gc