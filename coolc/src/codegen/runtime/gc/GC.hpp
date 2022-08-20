#pragma once

#include "Allocator.hpp"
#include "Marker.hpp"

namespace gc
{
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
        GcTypeNumber
    };

  protected:
    Allocator *_allocator;
    Marker *_marker;

    gc_address _heap_start;
    gc_address _heap_end;

    static GC *Gc;

  public:
    /**
     * @brief Construct a new GC
     */
    GC()
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

    virtual ~GC()
    {
    }
};

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
}; // namespace gc