#pragma once

#include "StackWalker.hpp"
#include "runtime/ObjectLayout.hpp"
#include <cassert>
#include <queue>

namespace gc
{
/**
 * @brief Marker marks live objects
 *
 */
class Marker
{
  protected:
    static Marker *MarkerObj;

    address _heap_start;
    address _heap_end;

    virtual void mark() = 0;

    inline bool is_heap_addr(address addr) const
    {
        return addr >= _heap_start && addr <= _heap_end;
    }

    virtual bool is_marked(ObjectLayout *object) const = 0;

    virtual void mark_unmarked_object(ObjectLayout *object) = 0;

  public:
    /**
     * @brief Construct a new Marker object
     *
     * @param heap_start Start of the heap to be marked
     * @param heap_end End of the heap to be marked
     */
    Marker(address heap_start, address heap_end);

    /**
     * @brief Mark arbitrary object
     *
     * @param root Object's address
     */
    virtual void mark_root(address *root) = 0;

    /**
     * @brief Mark live objects from roots
     *
     */
    virtual void mark_from_roots() = 0;

    /**
     * @brief Initialize global marker
     *
     * @param type GC Algo
     */
    static void init(GcType type);

    /**
     * @brief Destruct the global marker
     *
     */
    static void release();

    /**
     * @brief Get the global marker
     *
     * @return Marker* Global Marker
     */
    inline static Marker *marker()
    {
        return MarkerObj;
    }

    virtual ~Marker();
};

class MarkerFIFO : public Marker
{
  protected:
    std::queue<ObjectLayout *> _worklist;

    void mark() override;

    static void mark_root(void *obj, address *root, const address *meta);

    bool is_marked(ObjectLayout *object) const override;

    void mark_unmarked_object(ObjectLayout *object) override;

  public:
    MarkerFIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots() override;

    void mark_root(address *root) override;
};

class BitMapMarker : public MarkerFIFO
{
  public:
    typedef size_t BitMapWord;

    static constexpr int BITS_PER_BYTE = 8;
    static constexpr int BITS_PER_BIT_MAP_WORD = sizeof(BitMapWord) * BITS_PER_BYTE;
    static constexpr int BYTES_PER_BIT = 1;

  protected:
    std::vector<BitMapWord> _bitmap;

    static void mark_root(void *obj, address *root, const address *meta);

    void mark_unmarked_object(ObjectLayout *object) override;

  public:
    BitMapMarker(address heap_start, address heap_end);

    void mark_from_roots() override;

    void mark_root(address *root) override
    {
        MarkerFIFO::mark_root(root);
    }

    bool is_marked(ObjectLayout *object) const override;

    /**
     * @brief Check if bit is set
     *
     * @param bitnum Bit number
     * @return true if bit is set
     * @return false if bit is not set
     */
    inline bool is_bit_set(size_t bitnum) const
    {
        assert(bitnum / BITS_PER_BIT_MAP_WORD < _bitmap.size());
        return (_bitmap[bitnum / BITS_PER_BIT_MAP_WORD] & (1llu << (bitnum % BITS_PER_BIT_MAP_WORD))) != 0;
    }

    /**
     * @brief Get bit for the given byte
     *
     * @param byte Byte number
     * @return size_t Bit number
     */
    inline size_t byte_to_bit(address byte) const
    {
        return (size_t)((address)byte - _heap_start) / BYTES_PER_BIT;
    }

    /**
     * @brief Number of bits in this bitmap
     *
     * @return size_t Number of bits
     */
    inline size_t bits_num() const
    {
        return (_heap_end - _heap_start) / BYTES_PER_BIT + 1;
    }

    /**
     * @brief Clear bitmap
     *
     */
    void clear();
};
} // namespace gc