#pragma once

#include "object-desc.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <queue>
#include <vector>

extern bool LOGALOC;
extern bool LOGMARK;
extern bool LOGSWEEP;
extern bool LOGGING;
extern bool ZEROING;

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

#define guarantee_null(val) guarantee(val == NULL, #val " is not null!")

#define guarantee_not_null(val) guarantee(val != NULL, #val " is null!")

// other macro
#define UNIMPEMENTED(method)                                                                                           \
    std::cerr << "Unimplemented method: " method << std::endl;                                                         \
    abort();

// logging macros
#ifdef DEBUG
#define LOG_ALLOC(base, size)                                                                                          \
    if (LOGGING)                                                                                                       \
    {                                                                                                                  \
        if (LOGALOC)                                                                                                   \
            std::cerr << "Allocate object " << std::hex << (uint64_t)base << " of size " << std::dec << size           \
                      << std::endl;                                                                                    \
        _allocated_size += size;                                                                                       \
    }

#define LOG_MARK_ROOT(base)                                                                                            \
    if (LOGMARK)                                                                                                       \
        std::cerr << "Mark root object " << std::hex << (uint64_t)base << std::endl;

#define LOG_MARK(base)                                                                                                 \
    if (LOGMARK)                                                                                                       \
        std::cerr << "Mark object " << std::hex << (uint64_t)base << std::endl;

#define LOG_SWEEP(base, size)                                                                                          \
    if (LOGGING)                                                                                                       \
    {                                                                                                                  \
        if (LOGSWEEP)                                                                                                  \
            std::cerr << "Sweep object " << std::hex << (uint64_t)base << " of size " << std::dec << size              \
                      << std::endl;                                                                                    \
        _freed_size += size;                                                                                           \
    }

#define LOG_COLLECT()                                                                                                  \
    if (LOGGING)                                                                                                       \
        std::cerr << "Collected dead objects!\n" << std::endl;
#else
#define LOG_ALLOC(base, size)
#define LOG_MARK_ROOT(base)
#define LOG_MARK(base)
#define LOG_SWEEP(base, size)
#define LOG_COLLECT()
#endif // DEBUG

namespace gc
{

class StackRecord;
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

#ifdef DEBUG
    uint64_t _allocated_size; // collect allocated size
    uint64_t _freed_size;     // collect size of the freed objects

#endif // DEBUG

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
class ZeroGC : public GC
{
  protected:
    const size_t _heap_size;

    address _heap_start;
    address _heap_pos;

  public:
    ZeroGC(size_t heap_size);

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

    ~ZeroGC()
    {
        free(_heap_start);
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

    std::vector<address *> _objects;

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
     */
    __attribute__((always_inline)) void reg_root(address *obj)
    {
        _objects.push_back(obj);
    }

    /**
     * @brief Get vector of the roots
     *
     * @return Vector of the roots
     */
    inline std::vector<address *> &roots_unsafe()
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

#define WORKLIST_MAX_LEN 8192

/**
 * @brief Marker marks live objects
 *
 */
class Marker
{
  protected:
    address _heap_start;
    address _heap_end;

    void mark()
    {
        UNIMPEMENTED("mark");
    }

  public:
    /**
     * @brief Construct a new Marker object
     *
     * @param heap_start Start of the heap to be marked
     * @param heap_end End of the heap to be marked
     */
    Marker(address heap_start, address heap_end);

    /**
     * @brief Mark live objects from root
     *
     * @param sr Current StackRecord
     */
    void mark_from_roots(StackRecord *sr)
    {
        UNIMPEMENTED("mark_from_roots");
    }
};

class MarkerLIFO : public Marker
{
  private:
    /* "For a single-threaded collector, the work list could be implemented as a stack. This leads to a depthfirst
     * traversal of the graph. If mark bits are co-located with objects, it has the advantage that the
     * elements that are processed next are those that have been marked most recently, and hence are likely
     * to still be in the hardware cache." (c) The Garbage Collection Handbook, Richard Jones, p. 47
     */
    std::vector<objects::ObjectHeader *> _worklist; // TODO: maybe smth better?

    void mark();

  public:
    MarkerLIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};

class MarkerFIFO : public Marker
{
  protected:
    /* "Cher et al [2004] observe that the fundamental problem is that cache lines are fetched in a breadthfirst,
     * first-in, first-out (FIFO), order but the mark-sweep algorithm traverses the graph depth-first,
     * last-in, first-out (LIFO). Their solution is to insert a first-in, first-out queue in front of the mark stack."
     * (c) The Garbage Collection Handbook, Richard Jones, p. 56
     */
    std::queue<objects::ObjectHeader *> _worklist;

    void mark();

  public:
    MarkerFIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};

class MarkerEdgeFIFO : public MarkerFIFO
{
  private:
    /* Garner et al [2007] realised that mark’s tracing loop can be restructured
     * to offer greater opportunities for prefetching.
     * Instead of adding children to the work list only if they are unmarked,
     * this algorithm inserts the children of unmarked objects unconditionally.
     * (c) The Garbage Collection Handbook, Richard Jones, p. 57
     */

    void mark();

  public:
    MarkerEdgeFIFO(address heap_start, address heap_end) : MarkerFIFO(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};

// --------------------------------------- Mark-Sweep ---------------------------------------
// The Garbage Collection Handbook, Richard Jones: 2.1 The mark-sweep algorithm
template <class MarkerType> class MarkSweepGC : public ZeroGC
{
  protected:
    address _heap_end;

    MarkerType _mrkr;

    void sweep();

    // helpers for collection
    void free(address obj);
    address next_object(address obj);

    // helper for allocation
    address find_free_chunk(size_t size);

  public:
    MarkSweepGC(size_t heap_size);

    address allocate(objects::Klass *klass);

    void collect();
};
}; // namespace gc