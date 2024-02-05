#include "GC.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <cstring>

using namespace gc;

GC *GC::Gc = nullptr;

std::chrono::milliseconds GCStats::Phases[GCPhaseCount];
std::string GCStats::PhasesNames[GCPhaseCount] = {"ALLOCATE", "MARK    ", "COLLECT "};

GCStats::GCStats(GCPhase phase)
    : _phase(phase),
      _local_start(duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()))
{
}

GCStats::~GCStats()
{
    Phases[_phase] +=
        (duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - _local_start);
}

void GCStats::dump()
{
    for (int i = 0; i < GCPhaseCount; i++)
    {
        fprintf(stderr, "GC Phase %s: %s\n", PhasesNames[i].c_str(), printable_time(Phases[i].count()).c_str());
    }
}

ObjectLayout *GC::allocate(int tag, size_t size, void *disp_tab)
{
    size = allign(size);

    Allocator *alloca = Allocator::allocator();

    ObjectLayout *object = nullptr;

    {
        GCStats phase(GCStats::GCPhase::ALLOCATE);
        object = alloca->allocate(tag, size, disp_tab);
    }

    if (object == nullptr)
    {
#ifdef DEBUG
        if (TraceGCCycles)
        {
            fprintf(stderr, "GC Collect was invoked!\n");
        }
#endif // DEBUG

        collect();

        {
            GCStats phase(GCStats::GCPhase::ALLOCATE);
            object = alloca->allocate(tag, size, disp_tab);
        }
    }

    if (object == nullptr)
    {
        alloca->exit_with_error("cannot allocate memory for object!");
    }

    assert((address)object >= alloca->start() && (address)object < alloca->end() &&
           (address)object + object->_size <= alloca->end());

    return object;
}

ObjectLayout *GC::copy(const ObjectLayout *obj)
{
    ObjectLayout *new_obj = allocate(obj->_tag, obj->_size, obj->_dispatch_table);
    assert(new_obj);

    size_t size = std::min(obj->_size, new_obj->_size) - Allocator::HEADER_SIZE; // because of allignemnt
    memcpy(new_obj->fields_base(), obj->fields_base(), size);

    return new_obj;
}

void GC::init()
{
    switch (GCAlgo)
    {
    case ZEROGC:
        Gc = new ZeroGC();
        break;
#if defined(LLVM_SHADOW_STACK) || defined(LLVM_STATEPOINT_EXAMPLE)
    case MARKSWEEPGC:
        Gc = new MarkSweepGC();
        break;
    case THREADED_MC_GC:
        Gc = new ThreadedCompactionGC();
        break;
    case COMPRESSOR_GC:
        Gc = new CompressorGC();
        break;
    case SEMISPACE_COPYING_GC:
        Gc = new SemispaceCopyingGC();
        break; // don't have explicit mark phase

#endif // LLVM_SHADOW_STACK || LLVM_STATEPOINT_EXAMPLE
    default:
        fprintf(stderr, "cannot select GC!\n");
        abort();
        break;
    };
}

void GC::release()
{
    delete Gc;
    Gc = nullptr;

    if (PrintGCStatistics)
    {
        GCStats::dump();
    }
}