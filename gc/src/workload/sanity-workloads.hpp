#include "gc/gc-interface/gc.hpp"

template <class GCType, int heap_size> int sanity_workload()
{
    objects::Klass integer(1);
    size_t i_field = integer.offset(0);

    GCType gc(heap_size);

    {
        gc::StackRecord sr(&gc);

        address ia = gc.allocate(&integer);
        guarantee(ia != NULL, "allocation failed!");
        sr.reg_root(ia);

        gc.write(ia, i_field, 0xDEAD);
        guarantee(gc.template read<doubleword>(ia, i_field) == 0xDEAD, "wrong value!");
    }

    return 0;
}