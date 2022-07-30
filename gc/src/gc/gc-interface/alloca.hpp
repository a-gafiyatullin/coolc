#pragma once

#include "defines.hpp"
#include "object.hpp"
#include <cstdlib>
#include <new>

extern bool LOGALOC;
extern bool ZEROING;

// logging macros
#ifdef DEBUG
#define LOG_ALLOC(base, size)                                                                                          \
    if (LOGALOC)                                                                                                       \
        std::cerr << "Allocate object " << std::hex << (uint64_t)base << " of size " << std::dec << size << std::endl; \
    _allocated_size += size;
#else
#define LOG_ALLOC(base, size)
#endif // DEBUG

namespace allocator
{
/**
 * @brief Basic allocation class
 *
 */
class Alloca
{
  protected:
    const size_t _size;

    address _start;
    address _end;

    address _pos;

#ifdef DEBUG
    uint64_t _allocated_size; // collect allocated size
    uint64_t _freed_size;     // collect size of the freed objects

#endif // DEBUG
  public:
    Alloca(size_t size);

    /**
     * @brief Allocate memory for object
     *
     * @param klass Klass of the object
     * @return address Start of the object
     */
    address allocate(objects::Klass *klass);

    /**
     * @brief Free memory
     *
     * @param start Start of the memory
     */
    void free(address start)
    {
        UNIMPEMENTED("free");
    }

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

    /**
     * @brief Print allocation info
     *
     */
    void dump();

    ~Alloca();
};

class NextFitAlloca : public Alloca
{
  public:
    NextFitAlloca(size_t size);

    address allocate(objects::Klass *klass);

    void free(address start);
};

}; // namespace allocator