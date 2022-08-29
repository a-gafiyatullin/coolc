#include "Marker.hpp"
#include "codegen/runtime/gc/Allocator.hpp"

using namespace gc;

Marker *Marker::MarkerObj = nullptr;

Marker::Marker(address heap_start, address heap_end) : _heap_start(heap_start), _heap_end(heap_end)
{
}

Marker::~Marker()
{
}

void Marker::init()
{
    Allocator *alloca = Allocator::allocator();
    MarkerObj = new MarkerFIFO(alloca->start(), alloca->end());
}

void Marker::release()
{
    delete MarkerObj;
    MarkerObj = nullptr;
}

void MarkerFIFO::mark_root(void *obj, address *root, const address *meta)
{
    MarkerFIFO *mrkr = (MarkerFIFO *)obj;
    mrkr->mark_root(root);
}

void MarkerFIFO::mark_from_roots()
{
    assert(_worklist.empty());
    StackWalker::walker()->process_roots(this, &MarkerFIFO::mark_root);
}

void MarkerFIFO::mark_root(address *root)
{
    ObjectLayout *obj = (ObjectLayout *)(*root);
    if (obj && !obj->is_marked())
    {
        obj->set_marked();
        _worklist.push(obj);
        mark();
    }
}

void MarkerFIFO::mark()
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