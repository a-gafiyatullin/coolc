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
    Allocator *alloca = Allocator::allocator();

    ObjectLayout *object = nullptr;

    {
        GCStats phase(GCStats::GCPhase::ALLOCATE);
        object = alloca->allocate(tag, size, disp_tab);
    }

    if (object == nullptr)
    {
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

void GC::init(const GcType &type)
{
    switch (type)
    {
    case ZEROGC:
        Gc = new ZeroGC();
        break;
#ifdef LLVM_SHADOW_STACK
    case MARKSWEEPGC:
        Gc = new MarkSweepGC();
        break;
    case THREADED_MC_GC:
        Gc = new ThreadedCompactionGC();
        break;
#endif // LLVM_SHADOW_STACK
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