#include "gc/gc-interface/gc.hpp"

// --------------------------------------- StackRecord ---------------------------------------
gc::StackRecord::StackRecord(GC *gc) : _gc(gc), _parent(NULL)
{
    gc->_current_scope = this;
}

gc::StackRecord::~StackRecord()
{
    _gc->_current_scope = _parent;
}

gc::StackRecord::StackRecord(StackRecord *parent) : StackRecord(parent->_gc)
{
    _parent = parent;
}

// --------------------------------------- GCStatisticsScope ---------------------------------------
const char *gc::GCStatistics::GCStatisticsName[gc::GCStatistics::GCStatisticsTypeAmount] = {"ALLOCATION", "FULL_GC",
                                                                                            "EXECUTION"};

void gc::GCStatistics::print(GCStatisticsType type, const GCStatistics &stat, const char *delim)
{
    std::cout << GCStatisticsName[type] << ": " << stat.time() << delim;
}

void gc::GCStatistics::print_gc_stats(GC *gc)
{
    for (int i = GCStatistics::ALLOCATION; i < gc::GCStatistics::GCStatisticsTypeAmount - 1; i++)
    {
        auto stat_i = (gc::GCStatistics::GCStatisticsType)i;
        print(stat_i, gc->stat(stat_i), ", ");
    }

    auto last_stat = (gc::GCStatistics::GCStatisticsType)(gc::GCStatistics::GCStatisticsTypeAmount - 1);
    print(last_stat, gc->stat(last_stat), "\n");
}

gc::GCStatisticsScope::GCStatisticsScope(GCStatistics *stat)
    : _stat(stat), _start(duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch()))
{
}

void gc::GCStatisticsScope::flush()
{
    precision now = duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch());
    _stat->add_time(now - _start);
    _start = now;
}

gc::GCStatisticsScope::~GCStatisticsScope()
{
    _stat->add_time(duration_cast<precision>(std::chrono::system_clock::now().time_since_epoch()) - _start);
}

// --------------------------------------- ZeroGC ---------------------------------------
gc::GC::GC() : _exec(&_stat[GCStatistics::EXECUTION])
{
}

gc::GC::~GC()
{
    _exec.flush();
    GCStatistics::print_gc_stats(this);
}

gc::ZeroGC::ZeroGC(size_t heap_size, bool need_zeroing) : _heap_size(heap_size), _need_zeroing(need_zeroing)
{
    _heap_start = (address)malloc(heap_size);
    if (_heap_start == NULL)
    {
        throw std::bad_alloc();
    }
    _heap_pos = _heap_start;
}

address gc::ZeroGC::allocate(objects::Klass *klass)
{
    GCStatisticsScope scope(&_stat[GCStatistics::ALLOCATION]);

    size_t obj_size = klass->size();
    if (_heap_pos + obj_size >= _heap_start + _heap_size)
    {
        throw std::bad_alloc(); // for ZeroGC just throw an exception
    }

    address object = _heap_pos;
    _heap_pos += obj_size;

    objects::ObjectHeader *obj_header = (objects::ObjectHeader *)object;
    obj_header->_mark = 0;
    obj_header->_size = obj_size;
    obj_header->_tag = klass->type();

    if (_need_zeroing)
    {
        memset(object + sizeof(objects::ObjectHeader), 0, obj_size);
    }

    return object;
}

// --------------------------------------- Marker ---------------------------------------
void gc::Marker::mark_from_roots(StackRecord *sr)
{
    guarantee(_worklist.empty(), "_worklist is not empty!");

    while (sr)
    {
        for (address obj : sr->roots_unsafe())
        {
            objects::ObjectHeader *hdr = (objects::ObjectHeader *)obj;
            if (hdr && !hdr->is_marked())
            {
                hdr->set_marked();
                _worklist.push(hdr);
                mark();
            }
        }

        sr = sr->parent();
    }
}

void gc::Marker::mark()
{
    while (!_worklist.empty())
    {
        objects::ObjectHeader *hdr = _worklist.top();
        _worklist.pop();

        // fields of this objects are not heap pointers
        if (hdr->has_special_type())
        {
            continue;
        }

        int fields_cnt = hdr->field_cnt();
        address_fld fields = hdr->fields_base();
        for (int j = 0; j < fields_cnt; j++)
        {
            objects::ObjectHeader *child = (objects::ObjectHeader *)fields[j];
            if (child && !child->is_marked())
            {
                child->set_marked();
                _worklist.push(child);
            }
        }
    }
}