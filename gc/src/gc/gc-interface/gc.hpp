#pragma once

#include "alloca.hpp"
#include "mark.hpp"
#include <chrono>

extern bool LOGSWEEP;
extern bool LOGGING;

// some common macro for convience
#define ALLOCATE(type) gc.allocate(&type)
#define COLLECT() gc.collect()

#define WRITE(base, address, value) gc.write(base, address, value)
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

#define LOG_COLLECT_MSG(msg)                                                                                           \
    if (LOGGING)                                                                                                       \
        std::cerr << "Collection: " << msg << std::endl;

#else
#define LOG_SWEEP(base, size)
#define LOG_COLLECT()
#define LOG_COLLECT_MSG()
#endif // DEBUG

namespace gc
{

typedef std::chrono::milliseconds precision;

/**
 * @brief Class to gather GC Statistics
 *
 */
template <class ObjectHeaderType> class GCStatistics
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
    static void print_gc_stats(GC<ObjectHeaderType> *gc);
};

/**
 * @brief Helper for time calculation
 *
 */
template <class ObjectHeaderType> class GCStatisticsScope
{
  private:
    GCStatistics<ObjectHeaderType> *_stat;

    precision _start;

  public:
    /**
     * @brief Construct a new GCStatisticsScope and start time record
     *
     * @param stat GCStatistics to record
     */
    GCStatisticsScope(GCStatistics<ObjectHeaderType> *stat);

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
template <class ObjectHeaderType> class GC
{
    friend class StackRecord<ObjectHeaderType>;
    friend class GCStatistics<ObjectHeaderType>;

  protected:
    StackRecord<ObjectHeaderType> *_current_scope; // emulate stack
    GCStatistics<ObjectHeaderType> _stat[GCStatistics<ObjectHeaderType>::GCStatisticsTypeAmount];

    GCStatisticsScope<ObjectHeaderType> _exec; // collect exec time

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
    virtual address allocate(objects::Klass<ObjectHeaderType> *klass) = 0;

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
    inline const GCStatistics<ObjectHeaderType> &stat(
        typename GCStatistics<ObjectHeaderType>::GCStatisticsType type) const
    {
        assert(type < GCStatistics<ObjectHeaderType>::GCStatisticsTypeAmount);
        return _stat[type];
    }

    /**
     * @brief Destroy the GC and record stats
     *
     */
    ~GC();
};

// --------------------------------------- ZeroGC ---------------------------------------
template <template <class> class Allocator, class ObjectHeaderType> class ZeroGC : public GC<ObjectHeaderType>
{
  protected:
    Allocator<ObjectHeaderType> _alloca;
    using GC<ObjectHeaderType>::_stat;

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

    address allocate(objects::Klass<ObjectHeaderType> *klass) override;

    void collect() override
    {
    }
};

// --------------------------------------- Mark-Sweep ---------------------------------------
// The Garbage Collection Handbook, Richard Jones: 2.1 The mark-sweep algorithm
template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
class MarkSweepGC : public ZeroGC<Allocator, ObjectHeaderType>
{
  protected:
    using ZeroGC<Allocator, ObjectHeaderType>::_alloca;
    using ZeroGC<Allocator, ObjectHeaderType>::_stat;
    using ZeroGC<Allocator, ObjectHeaderType>::_current_scope;

    MarkerType<ObjectHeaderType> _mrkr;

    void sweep();

  public:
    MarkSweepGC(size_t heap_size);

    void collect() override;
};

// The Garbage Collection Handbook, Richard Jones: 3.2 The Lisp 2 algorithm
template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
class Lisp2GC : public ZeroGC<Allocator, ObjectHeaderType>
{
  protected:
    using ZeroGC<Allocator, ObjectHeaderType>::_alloca;
    using ZeroGC<Allocator, ObjectHeaderType>::_stat;
    using ZeroGC<Allocator, ObjectHeaderType>::_current_scope;

    MarkerType<ObjectHeaderType> _mrkr;

    /* 1. phase computeLocations.
     * The first pass over the heap (after marking) computes the location to which each live object will be
     * moved, and stores this address in the object’s forwardingAddress field
     */
    void compute_locations();

    /* 2. phase updateReferences.
     * The second pass updates the roots of mutator threads and
     * references in marked objects so that they refer to the new locations of their targets, using the
     * forwarding address stored in each about-to-be-relocated object’s header by the first pass.
     */
    void update_references();

    /* 3. phase relocate.
     * Finally, in the third pass, relocate moves each live (marked) object in a region to its new destination.
     */
    void relocate();

    // main compaction routine
    void compact();

  public:
    Lisp2GC(size_t heap_size);

    void collect() override;
};
}; // namespace gc