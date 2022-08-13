#include "gc/gc-interface/gc.hpp"

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
gc::MarkCompactGC<Allocator, MarkerType, ObjectHeaderType>::MarkCompactGC(size_t heap_size)
    : ZeroGC<Allocator, ObjectHeaderType>(heap_size), _mrkr(_alloca.start(), _alloca.end())
{
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::MarkCompactGC<Allocator, MarkerType, ObjectHeaderType>::collect()
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

            obj->unset_marked();
            _alloca.move(obj, dest);
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

// --------------------------------------- Jonkers’s threaded compactor ---------------------------------------
template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::ThreadedCompactionGC<Allocator, MarkerType, ObjectHeaderType>::thread(address *ref)
{
    static_assert(sizeof(ObjectHeaderType::_size) == sizeof(address), "sanity");

    address obj = *ref; // address of the object
    if (obj != NULL)
    {
        ObjectHeaderType *hdr = (ObjectHeaderType *)obj;
        size_t size = hdr->_size;
        hdr->_size = (size_t)ref;

        *ref = (address)size;
    }
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::ThreadedCompactionGC<Allocator, MarkerType, ObjectHeaderType>::update(address *obj, address addr)
{
    // if it is threaded object, so all references to it was orginized to linked list
    address temp = (address)((ObjectHeaderType *)obj)->_size;
    // if _size field contains a heap pointer - it is a start of the list
    while (_alloca.is_heap_addr(temp) || (temp >= _stack_start && temp <= _stack_end))
    {
        // we points to fields
        address next = *(address *)temp;
        *(address *)temp = addr; // restore address
        temp = next;
    }
    ((ObjectHeaderType *)obj)->_size = (size_t)temp; // restore original info
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::ThreadedCompactionGC<Allocator, MarkerType, ObjectHeaderType>::update_forward_references()
{
    // thread roots
    StackRecord<ObjectHeaderType> *scope = _current_scope;
    while (scope != NULL)
    {
        int roots_size = scope->roots_unsafe().size();
        for (int i = 0; i < roots_size; i++)
        {
            address *root = scope->roots_unsafe()[i];

            // hack to check for in update
            _stack_start = (address)std::min((address *)_stack_start, root);
            _stack_end = (address)std::max((address *)_stack_end, root);

            // book suggests "thread(*fld)". Mistake?
            thread(root);
        }

        scope = scope->parent();
    }

    address free = _alloca.start();
    address scan = free;

    address end = _alloca.end();
    // book suggests scan <= end. Mistake?
    while (scan < end)
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;

        // markword is ok, because we use _size for threading
        if (obj->is_marked())
        {
            update((address *)scan, free); // forward refs to scan set to free

            // fields of this objects are not heap pointers. Nothing to update
            if (!obj->has_special_type())
            {
                int fields_cnt = obj->field_cnt();
                address_fld fields = obj->fields_base();
                for (int j = 0; j < fields_cnt; j++)
                {
                    thread(fields + j);
                }
            }

            assert(obj->_size != 0);
            free = free + obj->_size;
        }
        // size field always is ok here
        scan = _alloca.next_object(scan);
    }
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::ThreadedCompactionGC<Allocator, MarkerType, ObjectHeaderType>::update_backward_references()
{
    address free = _alloca.start();
    address scan = free;

    address end = _alloca.end();
    while (scan < end)
    {
        ObjectHeaderType *obj = (ObjectHeaderType *)scan;

        // markword is ok, because we use _size for threading
        if (obj->is_marked())
        {
            // some threads was not processed
            update((address *)scan, free); // backward refs to scan set to free

            obj->unset_marked();
            _alloca.move(obj, free);
            free = free + obj->_size;
        }
#ifdef DEBUG
        else
        {
            _alloca.free(scan);
        }
#endif // DEBUG

        // size field always is ok here
        scan = _alloca.next_object(scan);
    }

    _alloca.force_alloc_pos(free);
}

template <template <class> class Allocator, template <class> class MarkerType, class ObjectHeaderType>
void gc::ThreadedCompactionGC<Allocator, MarkerType, ObjectHeaderType>::compact()
{
    update_forward_references();
    update_backward_references();
}