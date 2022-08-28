#include "Marker.hpp"

using namespace gc;

Marker::Marker(address heap_start, address heap_end) : _heap_start(heap_start), _heap_end(heap_end)
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

            if (obj && !obj->is_marked())
            {
                obj->set_marked();
                _worklist.push(obj);
                mark();
            }
        }
        r = r->_next;
    }
}

void ShadowStackMarkerFIFO::mark_runtime_root(address root)
{
    ObjectLayout *obj = (ObjectLayout *)root;
    if (obj && !obj->is_marked())
    {
        obj->set_marked();
        _worklist.push(obj);
        mark();
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
                if (size && !size->is_marked())
                {
                    size->set_marked();
                    _worklist.push(size);
                }
            }
            continue;
        }

        int fields_cnt = hdr->field_cnt();
        address *fields = hdr->fields_base();

        for (int j = 0; j < fields_cnt; j++)
        {
            ObjectLayout *child = (ObjectLayout *)fields[j];
            if (child && !child->is_marked())
            {
                child->set_marked();
                _worklist.push(child);
            }
        }
    }
}
