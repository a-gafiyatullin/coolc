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
    ObjectLayout *object = nullptr;

    {
        GCStats phase(GCStats::GCPhase::ALLOCATE);
        object = _allocator->allocate(tag, size, disp_tab);
    }

    if (object == nullptr)
    {
        collect();

        {
            GCStats phase(GCStats::GCPhase::ALLOCATE);
            object = _allocator->allocate(tag, size, disp_tab);
        }
    }

    if (object == nullptr)
    {
        _allocator->exit_with_error("cannot allocate memory for object!");
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

void GC::init(const GcType &type, const size_t &heap_size)
{
    switch (type)
    {
    case ZEROGC:
        Gc = new ZeroGC(heap_size);
        break;
    case MARKSWEEPGC:
        Gc = new MarkSweepGC(heap_size);
        break;
    default:
        Gc = new MarkSweepGC(heap_size);
        break;
    };
}

void GC::finish()
{
    delete Gc;
    Gc = nullptr;

    if (PrintGCStatistics)
    {
        GCStats::dump();
    }
}

// -------------------------------------- ZeroGC --------------------------------------
ZeroGC::ZeroGC(const size_t &size) : GC()
{
    _allocator = new Allocator(size);

    _heap_start = _allocator->start();
    _heap_end = _allocator->end();

    _marker = new ShadowStackMarkerFIFO(_heap_start, _heap_end);
}

ZeroGC::~ZeroGC()
{
    delete _allocator;
    delete _marker;
}