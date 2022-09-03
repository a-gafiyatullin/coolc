#include "StackWalker.hpp"

using namespace gc;

#ifndef LLVM_SHADOW_STACK
StackEntry *llvm_gc_root_chain = NULL;
#endif // !LLVM_SHADOW_STACK

StackWalker *StackWalker::Walker = nullptr;

void ShadowStackWalker::process_roots(void *obj, void (*visitor)(void *obj, address *root, const address *meta))
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

void StackWalker::init()
{
    Walker = new ShadowStackWalker;
}

void StackWalker::release()
{
    delete Walker;
    Walker = nullptr;
}