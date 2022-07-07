#pragma once

#include "object-desc.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <stack>
#include <vector>

// some common macro for convience
#define ALLOCATE(type) gc.allocate(&type)
#define COLLECT() gc.collect()

#define READ(base, offset, type) gc.template read<type>(base, offset)
#define READ_B(base, offset) READ(base, offset, byte)
#define READ_HW(base, offset) READ(base, offset, halfword)
#define READ_W(base, offset) READ(base, offset, word)
#define READ_DW(base, offset) READ(base, offset, doubleword)
#define READ_ADDRESS(base, offset) READ(base, offset, address)

#define WRITE(base, address, value) gc.write(base, address, value)

// some macro for testing
#define guarantee(cond, msg)                                                                                           \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        std::cerr << __FILE__ ":" << __LINE__ << ": condition (" #cond ") failed: " msg << std::endl;                  \
        abort();                                                                                                       \
    }

#define guarantee_eq(lv, rv) guarantee(lv == rv, #lv " and " #rv " are not equal!")

#define guarantee_ne(lv, rv) guarantee(lv != rv, #lv " and " #rv " are equal!")

// other macro
#define UNIMPEMENTED(method)                                                                                           \
    std::cerr << "Unimplemented method: " method << std::endl;                                                         \
    abort();

namespace gc
{

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

// --------------------------------------- ZeroGC ---------------------------------------
class ZeroGC : public GC
{
  protected:
    const size_t _heap_size;
    const bool _need_zeroing;

    address _heap_start;
    address _heap_pos;

  public:
    ZeroGC(size_t heap_size, bool need_zeroing);

    address allocate(objects::Klass *klass);

    template <class T> __attribute__((always_inline)) void write(address base, std::size_t offset, T src)
    {
        *((T *)(base + offset)) = src;
    }

    template <class T> __attribute__((always_inline)) T read(address base, std::size_t offset)
    {
        return *((T *)(base + offset));
    }

    void collect()
    {
    }
};

// --------------------------------------- StackRecord ---------------------------------------

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
    StackRecord(StackRecord *parent);

    /**
     * @brief Destroy the StackRecord and adjust GC state
     *
     */
    ~StackRecord();

    /**
     * @brief Add a new root
     *
     * @return Index of the local
     */
    __attribute__((always_inline)) int reg_root(address obj)
    {
        _objects.push_back(obj);
        return _objects.size() - 1;
    }

    /**
     * @brief Get root by the index
     *
     * @return Address of the object
     */
    __attribute__((always_inline)) address root(int i) const
    {
        assert(i < _objects.size());
        return _objects[i];
    }

    /**
     * @brief Get vector of the roots
     *
     * @return Vector of the roots
     */
    inline std::vector<address> &roots_unsafe()
    {
        return _objects;
    }

    /**
     * @brief Parental Stack Record
     *
     * @return StackRecord
     */
    inline StackRecord *parent() const
    {
        return _parent;
    }
};

/**
 * @brief Marker marks live objects
 *
 */
class Marker
{
  private:
    /* "For a single-threaded collector, the work list could be implemented as a stack. This leads to a depthfirst
     * traversal of the graph. If mark bits are co-located with objects, it has the advantage that the
     * elements that are processed next are those that have been marked most recently, and hence are likely
     * to still be in the hardware cache." (c) The Garbage Collection Handbook, Richard Jones, p. 47
     */
    std::stack<objects::ObjectHeader *> _worklist; // TODO: maybe smth better?

    void mark();

  public:
    /**
     * @brief Mark live objects from root
     *
     * @param sr Current StackRecord
     */
    void mark_from_roots(StackRecord *sr);
};

// --------------------------------------- Mark-Sweep ---------------------------------------
// The Garbage Collection Handbook, Richard Jones: 2.1 The mark-sweep algorithm
class MarkSweepGC : public ZeroGC
{
  protected:
    address _heap_end;

    Marker _mrkr;

    void sweep();

    // helpers for collection
    void free(address obj);
    address next_object(address obj);

    // helper for allocation
    address find_free_chunk(size_t size);

  public:
    MarkSweepGC(size_t heap_size, bool need_zeroing);

    address allocate(objects::Klass *klass);

    void collect();
};
}; // namespace gc