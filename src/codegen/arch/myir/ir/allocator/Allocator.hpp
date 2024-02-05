#pragma once

#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace allocator
{

// Objects of this class cannot be allocted in heap
class StackObject
{
  private:
    void *operator new(std::size_t size);

    void operator delete(void *ptr);

    void *operator new[](std::size_t size);

    void operator delete[](void *ptr);
};

// Allocator that grows and free memory in the end of scope
class LinearAllocator : public StackObject
{
  private:
    size_t _chunk_size;

    struct Chunk
    {
        char *_start;
        char *_pos;
    };

    std::list<Chunk> _chunks;

    void grow();

  public:
    LinearAllocator(int chunk_size, bool is_global = false);

    void *allocate(size_t size, size_t align);
    void deallocate(void *ptr) {}

    bool operator==(const LinearAllocator &rhs) const;

    ~LinearAllocator();
};

template <typename T, class Strategy> class IRAllocator
{
  public:
    template <typename U> struct rebind // NOLINT
    {
        using other = IRAllocator<U, Strategy>;
    };

    Strategy *_strategy;

    using value_type = T;

    IRAllocator(const IRAllocator &other) noexcept : _strategy(other._strategy) {}

    IRAllocator(Strategy *alloca) noexcept : _strategy(alloca) {}

    template <typename U> IRAllocator(const IRAllocator<U, Strategy> &other) noexcept : _strategy(other._strategy) {}

    T *allocate(std::size_t n) { return static_cast<T *>(_strategy->allocate(n * sizeof(T), alignof(T))); }

    void deallocate(void *ptr, std::size_t n) { _strategy->deallocate(ptr); }
};

template <typename T, typename U, class Strategy>
bool operator==(const IRAllocator<T, Strategy> &lhs, const IRAllocator<U, Strategy> &rhs)
{
    return lhs._strategy == rhs._strategy;
}

template <typename T, typename U, class Strategy>
bool operator!=(const IRAllocator<T, Strategy> &lhs, const IRAllocator<U, Strategy> &rhs)
{
    return !(lhs == rhs);
}
}; // namespace allocator

namespace myir
{
#define USE_CUSTOM_ALLOC

// default data types
#ifdef USE_CUSTOM_ALLOC
template <class T> using irallocator = allocator::IRAllocator<T, allocator::LinearAllocator>;
template <class Key, class T>
using irkvallocator = allocator::IRAllocator<std::pair<const Key, T>, allocator::LinearAllocator>;
#else
template <class T> using irallocator = std::allocator<T>;
template <class Key, class T> using irkvallocator = std::allocator<std::pair<const Key, T>>;
#endif

class IRObject
{
  protected:
    static allocator::LinearAllocator *Alloca;

    irallocator<void *> _alloc;

  public:
#ifdef USE_CUSTOM_ALLOC
    IRObject() : _alloc(Alloca) {}
#else
    IRObject() {}
#endif

    static constexpr size_t DEFAULT_CHUNK_SIZE = 10 * 1024 * 1024; // 10 MB

    static void set_alloca(allocator::LinearAllocator *alloca) { Alloca = alloca; }

#ifdef USE_CUSTOM_ALLOC
    void *operator new(std::size_t size) { return Alloca->allocate(size, alignof(std::max_align_t)); }
    void operator delete(void *ptr) { Alloca->deallocate(ptr); }
    void *operator new[](std::size_t size) { return Alloca->allocate(size, alignof(std::max_align_t)); }
    void operator delete[](void *ptr) { Alloca->deallocate(ptr); }
#endif
};

#define COMMAS ,

#ifdef USE_CUSTOM_ALLOC
#define ALLOC _alloc
#define ALLOCCOMMA COMMAS _alloc
#else
#define ALLOC
#define ALLOCCOMMA
#endif

template <class T> using irvector = std::vector<T, irallocator<T>>;

template <class T> using irlist = std::list<T, irallocator<T>>;

using irstring = std::basic_string<char, std::char_traits<char>, irallocator<char>>;

// for some reason unordered map constructor with allocator calls default constructor
template <class Key, class T> using irunordered_map = std::map<Key, T, std::less<Key>, irkvallocator<Key, T>>;

template <class Key> using irset = std::set<Key, std::less<Key>, irallocator<Key>>;

}; // namespace myir