#pragma once

#include "codegen/runtime/ObjectLayout.hpp"

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
    const size_t _size;

    gc_address _start; // heap start
    gc_address _end;   // heap end

    gc_address _pos; // current allocation position

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
    inline gc_address start() const
    {
        return _start;
    }

    /**
     * @brief Get the end of the heap
     *
     * @return address of the end of the heap
     */
    inline gc_address end() const
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
    inline bool is_heap_addr(gc_address addr) const
    {
        return addr >= _start && addr <= _end;
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
    void move(const ObjectLayout *src, gc_address dst);

    /**
     * @brief Hint for allocation
     *
     * @param pos Heap position
     */
    void force_alloc_pos(gc_address pos);

    /**
     * @brief Get next object
     *
     * @param obj Current object start
     * @return Next object
     */
    gc_address next_object(gc_address obj);
};
}; // namespace gc