#pragma once

#include "runtime/ObjectLayout.hpp"
#include <cassert>

namespace gc
{
/**
 * @brief Basic allocation class
 *
 */
class Allocator
{
  public:
    static const int HEADER_SIZE = sizeof(ObjectLayout);

  protected:
    static Allocator *AllocatorObj;

    const size_t _size;

    address _start; // heap start
    address _end;   // heap end

    address _pos; // current allocation position

#ifdef DEBUG
    uint64_t _allocated_size; // collect allocated size
    uint64_t _freed_size;     // collect size of the freed objects

#endif // DEBUG

    // real allocation/free methods
    virtual ObjectLayout *allocate_inner(int tag, size_t size, void *disp_tab);
    virtual void free_inner(ObjectLayout *obj)
    {
        assert(false);
    } // TODO: should not reach here

  public:
    /**
     * @brief Create a new Allocator with given heap size
     *
     * @param size Heap Size
     */
    Allocator(const size_t &size);

    /**
     * @brief Allocate a new object
     *
     * @param tag Object tag
     * @param size Object size
     * @param disp_tab Dispatch table
     * @return Pointer to the newly allocated object
     */
    ObjectLayout *allocate(int tag, size_t size, void *disp_tab);

    /**
     * @brief Free memory of the object
     *
     * @param obj Object to free
     */
    void free(ObjectLayout *obj);

    /**
     * @brief Get the start of the heap
     *
     * @return address of the start of the heap
     */
    inline address start() const
    {
        return _start;
    }

    /**
     * @brief Get the end of the heap
     *
     * @return address of the end of the heap
     */
    inline address end() const
    {
        return _end;
    }

#ifdef DEBUG
    /**
     * @brief Print allocation info
     *
     */
    void dump();
#endif // DEBUG

    /**
     * @brief Stop program execution
     *
     * @param error Error message
     */
    void exit_with_error(const char *error);

    /**
     * @brief Check if this address is heap addr
     *
     * @param addr Address to check
     * @return true if addr is from the heap
     * @return false if addr isn't from the heap
     */
    inline bool is_heap_addr(address addr) const
    {
        return addr >= _start && addr < _end;
    }

    /**
     * @brief Initialize global allocator
     *
     * @param size Heap Size
     */
    static void init(const size_t &size);

    /**
     * @brief Destruct the allocator
     *
     */
    static void release();

    /**
     * @brief Get the global allocator
     *
     * @return Allocator* Global allocator
     */
    inline static Allocator *allocator()
    {
        return AllocatorObj;
    }

    virtual ~Allocator();
};

class NextFitAllocator : public Allocator
{
  protected:
    ObjectLayout *allocate_inner(int tag, size_t size, void *disp_tab) override;

    void free_inner(ObjectLayout *obj) override;

  public:
    NextFitAllocator(const size_t &size);
    /**
     * @brief Move object
     *
     * @param src Source object
     * @param dst New location
     */
    void move(const ObjectLayout *src, address dst);

    /**
     * @brief Hint for allocation
     *
     * @param pos Heap position
     */
    void force_alloc_pos(address pos);

    /**
     * @brief Get next object
     *
     * @param obj Current object start
     * @return Next object
     */
    address next_object(address obj);
};

// SemispaceNextFitAllocator is a NextFitAllocator that swaps semispaces every gc cycle
class SemispaceNextFitAllocator : public NextFitAllocator
{
  protected:
    size_t _extend;

    address _orig_heap_start;
    address _orig_heap_end;

  public:
    SemispaceNextFitAllocator(const size_t &size);

    /**
     * @brief Swap semispaces
     *
     */
    void flip();

    /**
     * @brief Space where we allocate
     *
     * @return address tospace start
     */
    inline address tospace() const
    {
        return _start;
    }

    /**
     * @brief Space from where we evacuate
     *
     * @return address fromspace start
     */
    inline address fromspace() const
    {
        return _end != _orig_heap_end ? _end : _orig_heap_start;
    }

    /**
     * @brief Check if this address is heap addr (in any of the semispaces)
     *
     * @param addr Address to check
     * @return true if addr is from the heap
     * @return false if addr isn't from the heap
     */
    inline bool is_orig_heap_addr(address obj) const
    {
        return obj >= _orig_heap_start && obj < _orig_heap_end;
    }

    virtual ~SemispaceNextFitAllocator();
};
}; // namespace gc