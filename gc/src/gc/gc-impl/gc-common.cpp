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

// --------------------------------------- ZeroGC ---------------------------------------
gc::ZeroGC::ZeroGC(const size_t &heap_size) : _heap_size(heap_size)
{
    _heap_start = (address)malloc(heap_size);
    guarantee(_heap_start != NULL, "can't allocate memotry!");
    _heap_pos = _heap_start;
}

address gc::ZeroGC::allocate(objects::Klass *klass)
{
    size_t obj_size = klass->size();
    guarantee(_heap_pos + obj_size < _heap_start + _heap_size, "out of memory!");

    address object = _heap_pos;
    _heap_pos += obj_size;

    objects::ObjectHeader *obj_header = (objects::ObjectHeader *)object;
    obj_header->_mark = 0;
    obj_header->_size = obj_size;
    obj_header->_tag = 1;
    memset(object + sizeof(objects::ObjectHeader), 0, obj_size);

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