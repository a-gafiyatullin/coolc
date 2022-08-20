#include "Marker.hpp"

using namespace gc;

Marker::Marker(gc_address heap_start, gc_address heap_end) : _heap_start(heap_start), _heap_end(heap_end)
{
}

void ShadowStackMarkerFIFO::mark_from_roots()
{
    assert(_worklist.empty());

    StackEntry *r = llvm_gc_root_chain;

    while (r)
    {
        assert(r->_map->_num_meta == 0); // we don't use metadata

        int num_roots = r->_map->_num_roots;
        for (int i = 0; i < num_roots; i++)
        {
            ObjectLayout *obj = (ObjectLayout *)r->_roots[i];

            // we don't bother about non-heap objects
            if (obj && is_heap_addr((gc_address)obj) && !obj->is_marked())
            {
                obj->set_marked();
                _worklist.push(obj);
                mark();
            }
#ifdef DEBUG
            else if (obj)
            {
                assert(obj->has_special_type());
            }
#endif // DEBUG
        }
        r = r->_next;
    }
}

void ShadowStackMarkerFIFO::mark()
{
    while (!_worklist.empty())
    {
        ObjectLayout *hdr = _worklist.front();
        _worklist.pop();

        assert(hdr);
        if (hdr->has_special_type())
        {
            if (hdr->is_string())
            {
                StringLayout *str = (StringLayout *)hdr;
                IntLayout *size = str->_string_size;
                assert(size);                           // cannot be null
                assert(is_heap_addr((gc_address)size)); // has to be heap object
                if (!size->is_marked())
                {
                    size->set_marked();
                    _worklist.push(size);
                }
            }
            continue;
        }

        int fields_cnt = hdr->field_cnt();
        gc_address *fields = hdr->fields_base();

        for (int j = 0; j < fields_cnt; j++)
        {
            ObjectLayout *child = (ObjectLayout *)fields[j];
#ifdef DEBUG
            if (!(is_heap_addr((gc_address)child) || !child))
            {
                hdr->print();
            }
            assert(is_heap_addr((gc_address)child) || !child);
#endif // DEBUG
            if (child && !child->is_marked())
            {
                child->set_marked();
                _worklist.push(child);
            }
        }
    }
}
