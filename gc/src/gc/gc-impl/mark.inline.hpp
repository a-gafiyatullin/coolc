#pragma once

#include "gc/gc-interface/mark.hpp"

// --------------------------------------- MarkerLIFO ---------------------------------------
template <class ObjectHeaderType>
gc::Marker<ObjectHeaderType>::Marker(address heap_start, address heap_end)
    : _heap_start(heap_start), _heap_end(heap_end)
{
}

template <class ObjectHeaderType>
void gc::MarkerLIFO<ObjectHeaderType>::mark_from_roots(StackRecord<ObjectHeaderType> *sr)
{
    assert(_worklist.empty());
    _worklist.reserve(WORKLIST_MAX_LEN);

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            ObjectHeaderType *hdr = (ObjectHeaderType *)(*obj);
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

template <class ObjectHeaderType> void gc::MarkerLIFO<ObjectHeaderType>::mark()
{
    while (!_worklist.empty())
    {
        ObjectHeaderType *hdr = _worklist.back();
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
            ObjectHeaderType *child = (ObjectHeaderType *)fields[j];
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
template <class ObjectHeaderType>
void gc::MarkerFIFO<ObjectHeaderType>::mark_from_roots(StackRecord<ObjectHeaderType> *sr)
{
    assert(_worklist.empty());

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            ObjectHeaderType *hdr = (ObjectHeaderType *)(*obj);
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

template <class ObjectHeaderType> void gc::MarkerFIFO<ObjectHeaderType>::mark()
{
    while (!_worklist.empty())
    {
        ObjectHeaderType *hdr = _worklist.front();
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
            ObjectHeaderType *child = (ObjectHeaderType *)fields[j];
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
template <class ObjectHeaderType>
void gc::MarkerEdgeFIFO<ObjectHeaderType>::mark_from_roots(StackRecord<ObjectHeaderType> *sr)
{
    assert(_worklist.empty());

    while (sr)
    {
        for (address *obj : sr->roots_unsafe())
        {
            ObjectHeaderType *hdr = (ObjectHeaderType *)(*obj);
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

template <class ObjectHeaderType> void gc::MarkerEdgeFIFO<ObjectHeaderType>::mark()
{
    while (!_worklist.empty())
    {
        ObjectHeaderType *hdr = _worklist.front();
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
            ObjectHeaderType *child = (ObjectHeaderType *)fields[j];
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