#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

// This code is mainly taken from https://llvm.org/docs/GarbageCollection.html#the-shadow-stack-gc

/* The map for a single function's stack frame.  One of these is
 * compiled as constant data into the executable for each function.
 *
 * Storage of metadata values is elided if the %metadata parameter to
 * @llvm.gcroot is null.
 */
struct FrameMap
{
    int32_t _num_roots;   // Number of roots in stack frame.
    int32_t _num_meta;    // Number of metadata entries.  May be < NumRoots.
    const void *_meta[0]; // Metadata for each root.
};

/* A link in the dynamic shadow stack.  One of these is embedded in
 * the stack frame of each function on the call stack.
 */
struct StackEntry
{
    StackEntry *_next;    // Link to next stack entry (the caller's).
    const FrameMap *_map; // Pointer to constant FrameMap.
    void *_roots[0];      // Stack roots (in-place array).
};

/* The head of the singly-linked list of StackEntries. Functions push
 * and pop onto this in their prologue and epilogue.
 *
 * Since there is only a global list, this technique is not threadsafe.
 */
extern StackEntry *llvm_gc_root_chain; // NOLINT // Will be defined by LLVM in case of shadow-stack GC