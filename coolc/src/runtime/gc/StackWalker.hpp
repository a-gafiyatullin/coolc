#pragma once

#ifdef LLVM_SHADOW_STACK
#include "ShadowStack.hpp"
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
#include "stack-map/StackMap.hpp"
#endif // LLVM_STATEPOINT_EXAMPLE

#include "runtime/ObjectLayout.hpp"

namespace gc
{

class Marker;

// info to relocate derived pointers
struct DeriviedPtrRelocInfo
{
    address *_base_ptr_slot;    // where to find a new address of the base pointer
    address *_derived_ptr_slot; // where to write a new derived ptr
    int _offset;                // offset from the base pointer
};

class StackWalker
{
  protected:
    static StackWalker *Walker;

  public:
    /**
     * @brief Visit stack roots (base pointers only)
     *
     * @param obj Arbitrray object for visitor
     * @param visitor Visistor func
     * @param record_derived_ptrs Record derived pointer to fix them further
     */
    virtual void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                               bool records_derived_ptrs = false) = 0;

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

    /**
     * @brief Apply relocations to derived pointers
     * Uses relocation info from the last process_roots
     */
    virtual void fix_derived_pointers() = 0;

    virtual ~StackWalker()
    {
    }
};

#ifdef LLVM_SHADOW_STACK
class ShadowStackWalker : public StackWalker
{
  public:
    void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                       bool records_derived_ptrs = false) override;

    // shadow stack don't create derived pointer
    void fix_derived_pointers() override
    {
    }
};
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
class StackMapWalker : public StackWalker
{
  private:
    std::vector<DeriviedPtrRelocInfo> _derived_ptrs;

    address *next_sp(address *sp, const stackmap::AddrInfo *info);
    address *next_fp(address *fp, const stackmap::AddrInfo *info);
    address *ret_addr(address *sp, address *fp, const stackmap::AddrInfo *info);
    // find the first addrinfo for COOL frame
    const stackmap::AddrInfo *find_addrinfo_from_rt(address *sp, address *fp, const stackmap::StackMap *map);

  public:
    StackMapWalker();

    void process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                       bool records_derived_ptrs = false) override;

    void fix_derived_pointers() override;

    ~StackMapWalker();
};
#endif // LLVM_STATEPOINT_EXAMPLE

} // namespace gc