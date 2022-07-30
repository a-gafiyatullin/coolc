#include "gc/gc-impl/gc-common.inline.hpp"
#include "gc/gc-impl/mark-sweep/gc-mark-sweep.inline.hpp"
#include "gc/gc-interface/gc.hpp"

bool LOGSWEEP = false;
bool LOGGING = false;

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

// --------------------------------------- GC ---------------------------------------
gc::GC::GC() : _exec(&_stat[GCStatistics::EXECUTION])
{
}

gc::GC::~GC()
{
    _exec.flush();
    GCStatistics::print_gc_stats(this);
}