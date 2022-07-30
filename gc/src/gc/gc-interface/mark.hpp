#pragma once

#include "defines.hpp"
#include "stack.hpp"
#include <queue>

extern bool LOGMARK;

// logging macros
#ifdef DEBUG
#define LOG_MARK_ROOT(base)                                                                                            \
    if (LOGMARK)                                                                                                       \
        std::cerr << "Mark root object " << std::hex << (uint64_t)base << std::endl;

#define LOG_MARK(base)                                                                                                 \
    if (LOGMARK)                                                                                                       \
        std::cerr << "Mark object " << std::hex << (uint64_t)base << std::endl;
#else
#define LOG_MARK_ROOT(base)
#define LOG_MARK(base)
#endif // DEBUG

#define WORKLIST_MAX_LEN 8192

namespace gc
{

/**
 * @brief Marker marks live objects
 *
 */
class Marker
{
  protected:
    address _heap_start;
    address _heap_end;

    void mark()
    {
        UNIMPEMENTED("mark");
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
     * @brief Mark live objects from root
     *
     * @param sr Current StackRecord
     */
    void mark_from_roots(StackRecord *sr)
    {
        UNIMPEMENTED("mark_from_roots");
    }
};

class MarkerLIFO : public Marker
{
  private:
    /* "For a single-threaded collector, the work list could be implemented as a stack. This leads to a depthfirst
     * traversal of the graph. If mark bits are co-located with objects, it has the advantage that the
     * elements that are processed next are those that have been marked most recently, and hence are likely
     * to still be in the hardware cache." (c) The Garbage Collection Handbook, Richard Jones, p. 47
     */
    std::vector<objects::ObjectHeader *> _worklist; // TODO: maybe smth better?

    void mark();

  public:
    MarkerLIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};

class MarkerFIFO : public Marker
{
  protected:
    /* "Cher et al [2004] observe that the fundamental problem is that cache lines are fetched in a breadthfirst,
     * first-in, first-out (FIFO), order but the mark-sweep algorithm traverses the graph depth-first,
     * last-in, first-out (LIFO). Their solution is to insert a first-in, first-out queue in front of the mark stack."
     * (c) The Garbage Collection Handbook, Richard Jones, p. 56
     */
    std::queue<objects::ObjectHeader *> _worklist;

    void mark();

  public:
    MarkerFIFO(address heap_start, address heap_end) : Marker(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};

class MarkerEdgeFIFO : public MarkerFIFO
{
  private:
    /* Garner et al [2007] realised that mark’s tracing loop can be restructured
     * to offer greater opportunities for prefetching.
     * Instead of adding children to the work list only if they are unmarked,
     * this algorithm inserts the children of unmarked objects unconditionally.
     * (c) The Garbage Collection Handbook, Richard Jones, p. 57
     */

    void mark();

  public:
    MarkerEdgeFIFO(address heap_start, address heap_end) : MarkerFIFO(heap_start, heap_end)
    {
    }

    void mark_from_roots(StackRecord *sr);
};
} // namespace gc