#include "tests-support.hpp"

template <class GCType, class ObjectHeaderType> void sanity_trivial_workload(size_t heap_size)
{
    size_t i_field = INTOBJ.offset(0);

    GCType gc(heap_size);

    {
        gc::StackRecord sr(&gc);

        address ia = ALLOCATE(INTOBJ);
        guarantee_not_null(ia);

        sr.reg_root(&ia);

        COLLECT();

        WRITE(ia, i_field, (doubleword)0xDEAD);
        guarantee_eq(READ_DW(ia, i_field), 0xDEAD);
    }
}

template <class GCType, class ObjectHeaderType> void sanity_trivial_collect_workload(size_t heap_size)
{
    size_t dataf = LLNODE.offset(0);
    size_t nextf = LLNODE.offset(1);

    size_t intf = INTOBJ.offset(0);

    GCType gc(heap_size);
    gc::StackRecord sr(&gc);

    // create linked list of the integers:
    address root_node = ALLOCATE(LLNODE);
    address second_node = ALLOCATE(LLNODE);
    address third_node = ALLOCATE(LLNODE);

    WRITE(root_node, nextf, second_node);
    WRITE(second_node, nextf, third_node);
    WRITE(third_node, nextf, NULL);

    sr.reg_root(&root_node);

    address dead_obj = NULL;
    {
        gc::StackRecord sr1(&sr);

        address root_i = ALLOCATE(INTOBJ);
        address second_i = ALLOCATE(INTOBJ);
        address third_i = ALLOCATE(INTOBJ);

        WRITE(root_node, dataf, root_i);
        WRITE(second_node, dataf, second_i);
        WRITE(third_node, dataf, third_i);

        WRITE(root_i, intf, 0);
        WRITE(second_i, intf, 1);
        WRITE(third_i, intf, 2);

        dead_obj = ALLOCATE(INTOBJ);
        WRITE(dead_obj, intf, (doubleword)0xDEAD);
        sr1.reg_root(&dead_obj);

        COLLECT();
    }

    guarantee_eq(READ_DW(dead_obj, intf), 0xDEAD);

    COLLECT();

    second_node = READ_ADDRESS(root_node, nextf);
    guarantee_not_null(second_node);

    third_node = READ_ADDRESS(second_node, nextf);
    guarantee_not_null(third_node);

    address third_i = READ_ADDRESS(third_node, dataf);
    guarantee_not_null(third_i);

    doubleword value = READ_DW(third_i, intf);
    guarantee_ne(value, 3);
}