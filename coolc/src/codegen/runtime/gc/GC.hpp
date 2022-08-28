#pragma once

#include "Allocator.hpp"
#include "Marker.hpp"
#include <chrono>
#include <string>

namespace gc
{
class GCStats
{
  public:
    enum GCPhase
    {
        ALLOCATE,
        MARK,
        COLLECT, // Compact/Sweep and etc

        GCPhaseCount
    };

  private:
    static std::chrono::milliseconds Phases[GCPhaseCount];
    static std::string PhasesNames[GCPhaseCount];

    std::chrono::milliseconds _local_start; // start of the period
    GCPhase _phase;

  public:
    /**
     * @brief Create a local time measurement
     *
     * @param phase Phase to measure
     */
    GCStats(GCPhase phase);

    ~GCStats();

    /**
     * @brief Print timers
     *
     */
    static void dump();
};

/**
 * @brief Base class for all GCs
 *
 */
class GC
{
  public:
    enum GcType
    {
        ZEROGC, // Dummy GC that don't really collect garbage
        MARKSWEEPGC,
        THREADED_MC_GC,

        GcTypeNumber
    };

  protected:
    static const int HEADER_SIZE = sizeof(ObjectLayout);

    Allocator *_allocator;
    Marker *_marker;

    address _heap_start;
    address _heap_end;

    address _runtime_root; // need it to preserve allocations in runtime routines

    static GC *Gc;

  public:
    /**
     * @brief Construct a new GC
     */
    GC() : _runtime_root(NULL)
    {
    }

    /**
     * @brief Initialize GC
     *
     * @param type GC Type
     */
    static void init(const GcType &type, const size_t &heap_size);

    /**
     * @brief Finalize GC
     *
     */
    static void finish();

    /**
     * @brief Get GC
     *
     * @return GC
     */
    static GC *gc()
    {
        assert(Gc);
        return Gc;
    }

    /**
     * @brief Allocate a new object
     *
     * @param tag Object tag
     * @param size Object size
     * @param disp_tab Dispatch table
     * @return Pointer to the newly allocated object
     */
    virtual ObjectLayout *allocate(int tag, size_t size, void *disp_tab);

    /**
     * @brief Create a copy of the object
     *
     * @param obj Object to copy
     * @return New object
     */
    virtual ObjectLayout *copy(const ObjectLayout *obj);

    /**
     * @brief Collect garbage
     *
     */
    virtual void collect() = 0;

    /**
     * @brief Preserve temporary allocation
     *
     * @param root Address of an object
     */
    void set_runtime_root(address root)
    {
        _runtime_root = root;
    }

    /**
     * @brief Get preserved allocation address
     *
     * @return address of the preserved object
     */
    address runtime_root() const
    {
        return _runtime_root;
    }

    virtual ~GC()
    {
    }
};

// --------------------------------------- ZeroGC ---------------------------------------
/**
 * @brief Dummy GC that don't really collect garbage
 *
 */
class ZeroGC : public GC
{
  public:
    ZeroGC(const size_t &size);

    void collect() override
    {
    }

    ~ZeroGC();
};

// --------------------------------------- Mark-Sweep ---------------------------------------
/**
 * @brief Mark-and-Sweep GC
 *
 */
class MarkSweepGC : public GC
{
  protected:
    void sweep();

  public:
    MarkSweepGC(const size_t &heap_size);

    void collect() override;

    ~MarkSweepGC();
};

// --------------------------------------- Mark-Compact ---------------------------------------
class MarkCompactGC : public GC
{
  protected:
    virtual void compact() = 0;

  public:
    MarkCompactGC(const size_t &heap_size);

    void collect() override;

    ~MarkCompactGC();
};

// The Garbage Collection Handbook, Richard Jones: 3.3 Jonkers’s threaded compactor
class ThreadedCompactionGC : public MarkCompactGC
{
  protected:
    // fast check on stack roots
    address _stack_start;
    address _stack_end;

    // Jonkers requires two passes over the heap:

    // 1. phase updateForwardReferences
    // the first to thread references that point forward in the heap
    void update_forward_references();

    // 2. phase updateBackwardReferences
    // and the second to thread backward pointers
    void update_backward_references();

    // main compaction routine
    void compact() override;

    // thread a reference
    void thread(address *ref);

    // unthread all references, replacing with addr
    void update(address *obj, address addr);

  public:
    ThreadedCompactionGC(const size_t &heap_size) : MarkCompactGC(heap_size)
    {
    }
};
}; // namespace gc