#include "StackWalker.hpp"
#include "codegen/runtime/globals.hpp"
#include <cassert>

using namespace gc;

StackWalker *StackWalker::Walker = nullptr;

#ifdef LLVM_SHADOW_STACK
void ShadowStackWalker::process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                                      bool records_derived_ptrs)
{
    StackEntry *r = llvm_gc_root_chain;

    while (r)
    {
        assert(r->_map->_num_meta == 0); // we don't use metadata

        int num_roots = r->_map->_num_roots;
        for (int i = 0; i < num_roots; i++)
        {
            (*visitor)(obj, (address *)(r->_roots + i), NULL);
        }

        r = r->_next;
    }
}
#endif // LLVM_SHADOW_STACK

void StackWalker::init()
{
#ifdef LLVM_SHADOW_STACK
    Walker = new ShadowStackWalker;
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
    Walker = new StackMapWalker;
#endif // LLVM_STATEPOINT_EXAMPLE
}

void StackWalker::release()
{
#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
    delete Walker;
    Walker = nullptr;
#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE
}

#ifdef LLVM_STATEPOINT_EXAMPLE
StackMapWalker::StackMapWalker() : StackWalker()
{
    stackmap::StackMap::init();
}

StackMapWalker::~StackMapWalker()
{
    stackmap::StackMap::release();
}

void StackMapWalker::process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta),
                                   bool records_derived_ptrs)
{
    if (records_derived_ptrs)
    {
        _derived_ptrs.clear();
    }

    address *stacktop = (address *)_stack_pointer;
    address *frametop = (address *)_frame_pointer;
    assert(stacktop || frametop);

#ifdef DEBUG
    if (TraceStackWalker)
    {
        fprintf(stderr, "\nStack pointer: %p\n", stacktop);
        fprintf(stderr, "Frame pointer: %p\n", frametop);
    }
#endif // DEBUG

    auto *const stackmap = stackmap::StackMap::map();
    assert(stackmap);

    // get the top frame
    const auto *stackinfo = find_addrinfo_from_rt(stacktop, frametop, stackmap);
    assert(stackinfo);

    int i = 0;
    while (stackinfo)
    {
        for (const auto &offset : stackinfo->_offsets)
        {
            address *base_ptr_slot =
                (address *)((offset._base_reg == stackmap::DWARFRegNum::SP ? (address)stacktop : (address)frametop) +
                            offset._base_offset);

            address *derived_ptr_slot =
                (address *)((offset._der_reg == stackmap::DWARFRegNum::SP ? (address)stacktop : (address)frametop) +
                            offset._offset);

            if (base_ptr_slot == derived_ptr_slot)
            {
                (*visitor)(obj, base_ptr_slot, NULL);
            }
            else if (records_derived_ptrs)
            {
                _derived_ptrs.push_back({base_ptr_slot, derived_ptr_slot, (int)(*derived_ptr_slot - *base_ptr_slot)});
#ifdef DEBUG
                if (TraceStackWalker)
                {
                    fprintf(stderr, "Found derived ptr in %p, base ptr is in %p. ",
                            _derived_ptrs.back()._derived_ptr_slot, _derived_ptrs.back()._base_ptr_slot);
                    fprintf(stderr, "Derived ptr is %p, base is %p, offset = 0x%x\n",
                            *_derived_ptrs.back()._derived_ptr_slot, *_derived_ptrs.back()._base_ptr_slot,
                            _derived_ptrs.back()._offset);
                }
#endif // DEBUG
            }
        }

        address next_ret_addr = (address)ret_addr(stacktop, frametop, stackinfo);

        // go to the next frame
        stacktop = next_sp(stacktop, stackinfo);
        frametop = next_fp(frametop, stackinfo);

        stackinfo = stackmap->info(next_ret_addr);

#ifdef DEBUG
        if (TraceStackWalker && stackinfo)
        {
            fprintf(stderr, "%d: ret addr: %p, stack size 0x%x\n", i++, next_ret_addr, stackinfo->_stack_size);
        }
#endif // DEBUG
    }
}

void StackMapWalker::fix_derived_pointers()
{
    for (const auto &reloc : _derived_ptrs)
    {
        *(reloc._derived_ptr_slot) = *(reloc._base_ptr_slot) + reloc._offset;
    }
}

#ifdef __x86_64__
const stackmap::AddrInfo *StackMapWalker::find_addrinfo_from_rt(address *sp, address *fp, const stackmap::StackMap *map)
{
    assert(map);
    return map->info((address)sp[-1]);
}

address *StackMapWalker::next_sp(address *sp, const stackmap::AddrInfo *info)
{
    return (address *)((address)sp + info->_stack_size + sizeof(address));
}

address *StackMapWalker::next_fp(address *fp, const stackmap::AddrInfo *info)
{
    // TODO: check for -fno-omit-frame-pointer
    // normally we don't use fp
    return fp;
}

address *StackMapWalker::ret_addr(address *sp, address *fp, const stackmap::AddrInfo *info)
{
    return (address *)(next_sp(sp, info)[-1]);
}
#elif __aarch64__
#include <execinfo.h>

const stackmap::AddrInfo *StackMapWalker::find_addrinfo_from_rt(address *sp, address *fp, const stackmap::StackMap *map)
{
    static const int BUFF_SIZE = 10;
    assert(map);

    void *buffer[BUFF_SIZE];
    int filled = backtrace(buffer, BUFF_SIZE);

    for (int i = 0; i < BUFF_SIZE; i++)
    {
        const auto *stackinfo = map->info((address)buffer[i]);
#ifdef DEBUG
        if (TraceStackWalker)
        {
            fprintf(stderr, "Try found stackmap for %p\n", buffer[i]);
        }
#endif // DEBUG
        if (stackinfo)
        {
            return stackinfo;
        }
    }

    return nullptr;
}

address *StackMapWalker::next_sp(address *sp, const stackmap::AddrInfo *info)
{
    return (address *)((address)sp + info->_stack_size);
}

address *StackMapWalker::next_fp(address *fp, const stackmap::AddrInfo *info)
{
    return (address *)(fp[0]);
}

address *StackMapWalker::ret_addr(address *sp, address *fp, const stackmap::AddrInfo *info)
{
    return (address *)(fp[1]);
}
#endif // __aarch64__

#endif // LLVM_STATEPOINT_EXAMPLE
