#include "gc/gc-interface/mark.hpp"

bool LOGMARK = false;

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