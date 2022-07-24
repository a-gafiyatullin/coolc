#include "gc/gc-impl/mark-sweep/gc-mark-sweep.inline.hpp"
#include "gc/gc-interface/gc.hpp"

bool LOGALOC = false;
bool LOGMARK = false;
bool LOGSWEEP = false;
bool LOGGING = false;
bool ZEROING = false;

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
const char *gc::GCStatistics::GCStatisticsName[gc::GCStatistics::GCStatisticsTypeAmount] = {"GC_MARK", "GC_SWEEP",
                                                                                            "ALLOCATION", "EXECUTION"};

void gc::GCStatistics::print(GCStatisticsType type, const GCStatistics &stat, long long sub, const char *delim)
{
    std::cout << GCStatisticsName[type] << ": " << stat.time() - sub << delim;
}

void gc::GCStatistics::print_gc_stats(GC *gc)
{
    long long sum = 0;
    for (int i = GCStatistics::GC_MARK; i < gc::GCStatistics::GCStatisticsTypeAmount - 1; i++)
    {
        auto stat_i = (gc::GCStatistics::GCStatisticsType)i;
        print(stat_i, gc->stat(stat_i), sum, ", ");

        sum = gc->stat(stat_i).time();
    }

    auto last_stat = (gc::GCStatistics::GCStatisticsType)(gc::GCStatistics::GCStatisticsTypeAmount - 1);
    print(last_stat, gc->stat(last_stat), sum, "\n");

#ifdef DEBUG
    std::cout << "Allocated bytes: " << gc->_allocated_size << std::endl;
    std::cout << "Freed bytes: " << gc->_freed_size << std::endl;
#endif // DEBUG
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
gc::GC::GC()
    : _exec(&_stat[GCStatistics::EXECUTION])
#ifdef DEBUG
      ,
      _allocated_size(0), _freed_size(0)
#endif // DEBUG
{
}

gc::GC::~GC()
{
    _exec.flush();
    GCStatistics::print_gc_stats(this);
}

gc::ZeroGC::ZeroGC(size_t heap_size) : _heap_size(heap_size)
{
    _heap_start = (address)malloc(heap_size);
    if (_heap_start == NULL)
    {
        GCStatistics::print_gc_stats(this);
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
        GCStatistics::print_gc_stats(this);
        throw std::bad_alloc(); // for ZeroGC just throw an exception
    }

    address object = _heap_pos;
    _heap_pos += obj_size;

    objects::ObjectHeader *obj_header = (objects::ObjectHeader *)object;
    obj_header->_mark = 0;
    obj_header->_size = obj_size;
    obj_header->_tag = klass->type();

    obj_header->zero_fields();

    LOG_ALLOC(object, obj_header->_size);

    return object;
}

// --------------------------------------- MarkerLIFO ---------------------------------------
gc::Marker::Marker(address heap_start, address heap_end) : _heap_start(heap_start), _heap_end(heap_end)
{
}

void gc::MarkerLIFO::mark_from_roots(StackRecord *sr)
{
    assert(_worklist.empty());
    _worklist.reserve(WORKLIST_MAX_LEN);

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            objects::ObjectHeader *hdr = (objects::ObjectHeader *)(*obj);
            assert((address)hdr >= _heap_start && (address)hdr < _heap_end || !hdr);
            if (hdr && !hdr->is_marked())
            {
                hdr->set_marked();
                LOG_MARK_ROOT(hdr);
                _worklist.push_back(hdr);
                mark();
            }
        }

        sr = sr->parent();
    }
}

void gc::MarkerLIFO::mark()
{
    while (!_worklist.empty())
    {
        objects::ObjectHeader *hdr = _worklist.back();
        _worklist.pop_back();

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
#ifdef DEBUG
            if (!((address)child >= _heap_start && (address)child < _heap_end || !child))
            {
                hdr->print();
            }
            assert((address)child >= _heap_start && (address)child < _heap_end || !child);
#endif // DEBUG
            if (child && !child->is_marked())
            {
                child->set_marked();
                LOG_MARK(child);
                _worklist.push_back(child);
                //__builtin_prefetch(child, 1);
            }
        }
    }
}

// --------------------------------------- MarkerFIFO ---------------------------------------
void gc::MarkerFIFO::mark_from_roots(StackRecord *sr)
{
    assert(_worklist.empty());

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            objects::ObjectHeader *hdr = (objects::ObjectHeader *)(*obj);
            assert((address)hdr >= _heap_start && (address)hdr < _heap_end || !hdr);
            if (hdr && !hdr->is_marked())
            {
                hdr->set_marked();
                LOG_MARK_ROOT(hdr);
                _worklist.push(hdr);
                mark();
            }
        }

        sr = sr->parent();
    }
}

void gc::MarkerFIFO::mark()
{
    while (!_worklist.empty())
    {
        objects::ObjectHeader *hdr = _worklist.front();
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
#ifdef DEBUG
            if (!((address)child >= _heap_start && (address)child < _heap_end || !child))
            {
                hdr->print();
            }
            assert((address)child >= _heap_start && (address)child < _heap_end || !child);
#endif // DEBUG
            if (child && !child->is_marked())
            {
                child->set_marked();
                LOG_MARK(child);
                _worklist.push(child);
            }
        }
    }
}

// --------------------------------------- MarkerEdgeFIFO ---------------------------------------
void gc::MarkerEdgeFIFO::mark_from_roots(StackRecord *sr)
{
    assert(_worklist.empty());

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            objects::ObjectHeader *hdr = (objects::ObjectHeader *)(*obj);
            assert((address)hdr >= _heap_start && (address)hdr < _heap_end || !hdr);
            if (hdr)
            {
                _worklist.push(hdr);
                mark();
            }
        }

        sr = sr->parent();
    }
}

void gc::MarkerEdgeFIFO::mark()
{
    while (!_worklist.empty())
    {
        objects::ObjectHeader *hdr = _worklist.front();
        _worklist.pop();

        if (hdr->is_marked())
        {
            continue;
        }

        hdr->set_marked();
        LOG_MARK(hdr);

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
#ifdef DEBUG
            if (!((address)child >= _heap_start && (address)child < _heap_end || !child))
            {
                hdr->print();
            }
            assert((address)child >= _heap_start && (address)child < _heap_end || !child);
#endif // DEBUG
            if (child)
            {
                _worklist.push(child);
            }
        }
    }
}