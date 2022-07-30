#pragma once

#include "alloca.hpp"
#include "mark.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>

extern bool LOGSWEEP;
extern bool LOGGING;

// some common macro for convience
#define ALLOCATE(type) gc.allocate(&type)
#define COLLECT() gc.collect()

#define READ(base, offset, type) gc.template read<type>(base, offset)
#define READ_B(base, offset) READ(base, offset, byte)
#define READ_HW(base, offset) READ(base, offset, halfword)
#define READ_W(base, offset) READ(base, offset, word)
#define READ_DW(base, offset) READ(base, offset, doubleword)
#define READ_ADDRESS(base, offset) READ(base, offset, address)

// logging macros
#ifdef DEBUG
#define LOG_SWEEP(base, size)                                                                                          \
    if (LOGSWEEP)                                                                                                      \
        std::cerr << "Sweep object " << std::hex << (uint64_t)base << " of size " << std::dec << size << std::endl;

#define LOG_COLLECT()                                                                                                  \
    if (LOGGING)                                                                                                       \
        std::cerr << "Collected dead objects!\n" << std::endl;
#else
#define LOG_SWEEP(base, size)
#define LOG_COLLECT()
#endif // DEBUG

namespace gc
{

class GC;

typedef std::chrono::milliseconds precision;

/**
 * @brief Class to gather GC Statistics
 *
 */
class GCStatistics
{
  private:
    precision _time;

  public:
    /**
     * @brief Construct a new GCStatistics object and zero time
     *
     */
    GCStatistics() : _time(0)
    {
    }

    enum GCStatisticsType
    {
        GC_MARK,
        GC_SWEEP,
        ALLOCATION,
        EXECUTION,
        GCStatisticsTypeAmount
    };

    static const char *GCStatisticsName[GCStatisticsTypeAmount];

    /**
     * @brief Add time
     *
     * @param time Time
     */
    inline void add_time(const precision &time)
    {
        _time += time;
    }

    /**
     * @brief Get gathered time
     *
     * @return Time
     */
    inline long long time() const
    {
        return _time.count();
    }

    /**
     * @brief Print statistics
     *
     * @param type Type of the statistics
     * @param stat Statistics object
     * @param sub Subtract value from stat
     * @param delim Delimeter
     */
    static void print(GCStatisticsType type, const GCStatistics &stat, long long sub, const char *delim);

    /**
     * @brief Print GC Stats
     *
     * @param gc GC
     */
    static void print_gc_stats(GC *gc);
};

/**
 * @brief Helper for time calculation
 *
 */
class GCStatisticsScope
{
  private:
    GCStatistics *_stat;

    precision _start;

  public:
    /**
     * @brief Construct a new GCStatisticsScope and start time record
     *
     * @param stat GCStatistics to record
     */
    GCStatisticsScope(GCStatistics *stat);

    /**
     * @brief Save and reset time record
     *
     */
    void flush();

    /**
     * @brief Destroy the GCStatisticsScope and finish time record
     *
     */
    ~GCStatisticsScope();
};

/**
 * @brief Base class for all GCs
 *
 */
class GC
{
    friend class StackRecord;
    friend class GCStatistics;

  protected:
    StackRecord *_current_scope; // emulate stack
    GCStatistics _stat[GCStatistics::GCStatisticsTypeAmount];

    GCStatisticsScope _exec; // collect exec time

  public:
    /**
     * @brief Construct a new GC and init stats
     *
     */
    GC();
    /**
     * @brief Allocate object of the klass
     *
     * @param klass Klass handle
     * @return address to the start of the object
     */
    virtual address allocate(objects::Klass *klass) = 0;

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
    virtual void collect() = 0;

    /**
     * @brief Get GC Statistics
     *
     * @param type Type of the statistics
     * @return GC Statistics
     */
    inline const GCStatistics &stat(GCStatistics::GCStatisticsType type) const
    {
        assert(type < GCStatistics::GCStatisticsTypeAmount);
        return _stat[type];
    }

    /**
     * @brief Destroy the GC and record stats
     *
     */
    ~GC();
};

// --------------------------------------- ZeroGC ---------------------------------------
template <class Allocator> class ZeroGC : public GC
{
  protected:
    Allocator _alloca;

  public:
    ZeroGC(size_t heap_size);

    template <class T> __attribute__((always_inline)) void write(address base, std::size_t offset, T src)
    {
        *((T *)(base + offset)) = src;
    }

    template <class T> __attribute__((always_inline)) T read(address base, std::size_t offset)
    {
        return *((T *)(base + offset));
    }

    address allocate(objects::Klass *klass) override;

    void collect() override
    {
    }
};

// --------------------------------------- Mark-Sweep ---------------------------------------
// The Garbage Collection Handbook, Richard Jones: 2.1 The mark-sweep algorithm
template <class Allocator, class MarkerType> class MarkSweepGC : public ZeroGC<Allocator>
{
  protected:
    using ZeroGC<Allocator>::_alloca;
    using ZeroGC<Allocator>::_stat;
    using ZeroGC<Allocator>::_current_scope;

    MarkerType _mrkr;

    void sweep();

    // helpers for collection
    address next_object(address obj);

  public:
    MarkSweepGC(size_t heap_size);

    void collect() override;
};
}; // namespace gc