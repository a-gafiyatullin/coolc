#pragma once

#include "StackWalker.hpp"
#include "codegen/runtime/ObjectLayout.hpp"
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

  public:
    MarkerFIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots() override;

    void mark_root(address *root) override;
};
} // namespace gc