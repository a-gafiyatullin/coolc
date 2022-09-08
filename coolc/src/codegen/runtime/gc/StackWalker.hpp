#pragma once

#ifdef LLVM_SHADOW_STACK
#include "ShadowStack.hpp"
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
#include "stack-map/StackMap.hpp"
#endif // LLVM_STATEPOINT_EXAMPLE

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

#ifdef LLVM_SHADOW_STACK
class ShadowStackWalker : public StackWalker
{
  public:
    void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta)) override;
};
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
class StackMapWalker : public StackWalker
{
  public:
    StackMapWalker();

    void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta)) override;

    ~StackMapWalker();
};
#endif // LLVM_STATEPOINT_EXAMPLE

} // namespace gc