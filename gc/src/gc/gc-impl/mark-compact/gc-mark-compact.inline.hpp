#include "gc/gc-interface/gc.hpp"

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::Lisp2GC(size_t heap_size)
    : ZeroGC<Allocator, ObjectHeaderType>(heap_size), _mrkr(_alloca.start(), _alloca.end())
{
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::collect()
{
    GCStatisticsScope scope(&_stat[GCStatistics<ObjectHeaderType>::GC_SWEEP]);

    {
        GCStatisticsScope scope(&_stat[GCStatistics<ObjectHeaderType>::GC_MARK]);
        _mrkr.mark_from_roots(_current_scope);
    }

    compact();

    LOG_COLLECT();
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::compact()
{
    compute_locations();
    update_references();
    relocate();
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::compute_locations()
{
    static_assert(sizeof(ObjectHeaderType::_mark) == sizeof(address), "sanity");

    address scan = _alloca.start();
    address end = _alloca.end();

    address free = scan;

    while (scan < end)
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;
        if (obj->is_marked())
        {
            obj->_mark = free;
            free = free + obj->_size;
        }
        scan = _alloca.next_object(scan);
    }
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::update_references()
{
    // update roots
    StackRecord<ObjectHeaderType> *scope = _current_scope;
    while (scope != NULL)
    {
        int roots_size = scope->roots_unsafe().size();
        for (int i = 0; i < roots_size; i++)
        {
            address *root = scope->roots_unsafe()[i];
            if (*root)
            {
                ObjectHeaderType *hdr = (ObjectHeaderType *)(*root);
                (*root) = hdr->_mark;
            }
        }

        scope = scope->parent();
    }

    // update marked objects
    address scan = _alloca.start();
    address end = _alloca.end();

    while (scan < end)
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;

        // fields of this objects are not heap pointers. Nothing to update
        if (obj->is_marked() && !obj->has_special_type())
        {
            int fields_cnt = obj->field_cnt();
            address_fld fields = obj->fields_base();
            for (int j = 0; j < fields_cnt; j++)
            {
                ObjectHeaderType *child = (ObjectHeaderType *)fields[j];

                if (child)
                {
                    fields[j] = child->_mark;
                }
            }
        }
        scan = _alloca.next_object(scan);
    }
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::Lisp2GC<Allocator, MarkerType, ObjectHeaderType>::relocate()
{
    address start = _alloca.start();
    address scan = start;

    address end = _alloca.end();

    address dest = scan;
    while (scan < end)
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;
        if (obj->is_marked())
        {
            dest = obj->_mark;
            assert(dest >= start && dest + obj->_size <= end && dest <= scan);

            _alloca.move(obj, dest);
            ((ObjectHeaderType *)dest)->unset_marked();
        }
#ifdef DEBUG
        else
        {
            _alloca.free(scan);
        }
#endif // DEBUG
        scan = _alloca.next_object(scan);
    }

    size_t dest_size = ((ObjectHeaderType *)dest)->_size;

    assert(dest >= start && dest + dest_size <= end && dest_size >= sizeof(ObjectHeaderType));
    _alloca.force_alloc_pos(dest + dest_size);
}