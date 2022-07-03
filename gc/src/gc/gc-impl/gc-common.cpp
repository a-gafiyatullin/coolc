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

gc::StackRecord::StackRecord(GC *gc, StackRecord *parent) : StackRecord(gc)
{
    assert(parent != NULL);
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
    obj_header->_tag = 0;
    memset(object + sizeof(objects::ObjectHeader), 0, obj_size);

    return object;
}