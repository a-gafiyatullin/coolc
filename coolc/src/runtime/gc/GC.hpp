#pragma once

#include "Allocator.hpp"
#include "Marker.hpp"
#include <chrono>
#include <unordered_set>

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
  protected:
    static const int HEADER_SIZE = sizeof(ObjectLayout);

    std::vector<address *> _runtime_roots; // need it to preserve allocations in runtime routines

    static GC *Gc;

  public:
    /**
     * @brief Initialize GC
     *
     * @param type GC Type
     */
    static void init(const GcType &type);

    /**
     * @brief Finalize GC
     *
     */
    static void release();

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
     * @param root Address of a slot with an object
     */
    inline void add_runtime_root(address *root)
    {
        _runtime_roots.push_back(root);
    }

    /**
     * @brief Clean list of preserved objects
     *
     */
    inline void clean_runtime_roots()
    {
        _runtime_roots.clear();
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
    void collect() override
    {
    }
};

#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
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
    void collect() override;
};

// --------------------------------------- Mark-Compact ---------------------------------------
class MarkCompactGC : public GC
{
  protected:
    virtual void compact() = 0;

  public:
    void collect() override;
};

// The Garbage Collection Handbook, Richard Jones: 3.3 Jonkers’s threaded compactor
// Some other compactors ans why i don't use them:
//    1. Edwards’s Two-Finger algorithm. Can't use because it compacts regions containing objects of a fixed size.
//    2. The Lisp 2 algorithm. Can't use because it requires an additional full-slot field
//       in every object header to store the address to which the object is to be moved.
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

    // stack walker helpers
    static void thread_root(void *obj, address *root, const address *meta);
};

// The Garbage Collection Handbook, Richard Jones: 3.4 One-pass algorithms
class CompressorGC : public MarkCompactGC
{
  protected:
    std::vector<size_t> _offsets;

    // roots in stackmap can be not unique.
    // update_stack_root is a destructive operation and after the first update it can try to update already updated root
    // that is wrong
    std::unordered_set<address *> _was_updated;

    static constexpr size_t BITS_IN_BLOCK = 256;

    // 1. The computeLocations routine passes over the mark bit vector to produce the offset vector.
    void compute_locations();

    // 2. The updateReferencesRelocate relocates objects and updates references in a single pass.
    void update_references_relocate();

    // calculate a new address for the given object
    address new_address(address old);

    // stack walker helpers
    static void update_stack_root(void *obj, address *root, const address *meta);

    // main compaction routine
    void compact() override;
};

#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE

}; // namespace gc