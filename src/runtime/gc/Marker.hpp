#pragma once

#include "StackWalker.hpp"
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

    inline bool is_heap_addr(address addr) const { return addr >= _heap_start && addr <= _heap_end; }

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
     */
    static void init();

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
    inline static Marker *marker() { return MarkerObj; }

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
    MarkerFIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end) {}

    void mark_from_roots() override;

    void mark_root(address *root) override;
};

class BitMapMarker : public MarkerFIFO
{
  protected:
    typedef size_t BitMapWord;

    static constexpr int BITS_PER_BYTE = 8;
    static constexpr int BITS_PER_BIT_MAP_WORD = sizeof(BitMapWord) * BITS_PER_BYTE;
    static constexpr int BYTES_PER_BIT = 8;

    std::vector<BitMapWord> _bitmap;

    static void mark_root(void *obj, address *root, const address *meta);

    void mark_unmarked_object(ObjectLayout *object) override;

  public:
    BitMapMarker(address heap_start, address heap_end);

    void mark_from_roots() override;

    void mark_root(address *root) override { MarkerFIFO::mark_root(root); }

    bool is_marked(ObjectLayout *object) const override;

    /**
     * @brief Check if bit is set
     *
     * @param bit Bit index
     * @return true if bit is set
     * @return false if bit is not set
     */
    inline bool is_bit_set(size_t bit) const
    {
        assert(bit / BITS_PER_BIT_MAP_WORD < _bitmap.size());
        return (_bitmap[bit / BITS_PER_BIT_MAP_WORD] & (1llu << (bit % BITS_PER_BIT_MAP_WORD))) != 0;
    }

    /**
     * @brief Get bit for the given byte
     *
     * @param byte Byte number
     * @return size_t Bit number
     */
    inline size_t byte_to_bit(address byte) const { return (size_t)((address)byte - _heap_start) / BYTES_PER_BIT; }

    /**
     * @brief Get the first byte that is represented by the given bit
     *
     * @param bit Bit index in bitmap
     * @return size_t Byte offset from the heap start
     */
    inline size_t bit_to_byte(size_t bit) const { return bit * BYTES_PER_BIT; }

    /**
     * @brief Get bitmap word number by the byte number
     *
     * @param byte Byte number
     * @return size_t Bitmap word number
     */
    inline size_t byte_to_word_num(address byte) const { return byte_to_bit(byte) / BITS_PER_BIT_MAP_WORD; }

    /**
     * @brief Get bitmap word by index
     *
     * @param idx Word index
     * @return const BitMapWord& Word
     */
    inline const BitMapWord &word(size_t idx) const { return _bitmap[idx]; }

    /**
     * @brief Number of words in bitmap
     *
     * @return size_t Number of words in bitmap
     */
    inline size_t words_num() const { return _bitmap.size(); }

    /**
     * @brief Get number of bits in the given amount of words
     *
     * @param word Number of words
     * @return size_t Number of bits
     */
    inline size_t word_to_bit(size_t words) const { return words * BITS_PER_BIT_MAP_WORD; }

    /**
     * @brief Number of bits in this bitmap
     *
     * @return size_t Number of bits
     */
    inline size_t bits_num() const
    {
        assert((_heap_end - _heap_start) % BYTES_PER_BIT == 0);
        return (_heap_end - _heap_start) / BYTES_PER_BIT;
    }

    /**
     * @brief Clear bitmap
     *
     */
    void clear();
};
} // namespace gc
