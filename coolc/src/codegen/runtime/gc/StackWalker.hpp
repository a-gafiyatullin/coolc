#pragma once

#include "ShadowStack.hpp"
#include "codegen/runtime/ObjectLayout.hpp"

namespace gc
{

class Marker;

class StackWalker
{
  protected:
    static StackWalker *Walker;

  public:
    /**
     * @brief Visit stack roots
     *
     * @param obj Arbitrray object for visitor
     * @param visitor Visistor func
     */
    virtual void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta)) = 0;

    /**
     * @brief Initialize global stack walker
     *
     */
    static void init();

    /**
     * @brief Destruct the global stack walker
     *
     */
    static void release();

    /**
     * @brief Get the global stack walker
     *
     * @return StackWalker* Global Stack Walker
     */
    inline static StackWalker *walker()
    {
        return Walker;
    }

    virtual ~StackWalker()
    {
    }
};

class ShadowStackWalker : public StackWalker
{
  public:
    void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta)) override;
};

} // namespace gc