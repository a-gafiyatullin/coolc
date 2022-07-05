#include "gc/gc-interface/gc.hpp"

template <class GCType, int heap_size> int sanity_workload_trivial()
{
    objects::Klass integer(1, objects::INTEGER);
    size_t i_field = integer.offset(0);

    GCType gc(heap_size);

    {
        gc::StackRecord sr(&gc);

        address ia = gc.allocate(&integer);
        guarantee(ia != NULL, "allocation failed!");

        gc.collect();

        int slot = sr.reg_root(ia);
        ia = sr.root(slot);

        gc.write(ia, i_field, 0xDEAD);
        guarantee(gc.template read<doubleword>(ia, i_field) == 0xDEAD, "wrong value!");
    }

    return 0;
}

template <class GCType, int heap_size> int sanity_workload_trivial_collect()
{
    objects::Klass linked_list_node(2, objects::OTHER);
    size_t dataf = linked_list_node.offset(0);
    size_t nextf = linked_list_node.offset(1);

    objects::Klass integer(1, objects::INTEGER);
    size_t intf = integer.offset(0);

    GCType gc(heap_size);
    gc::StackRecord sr(&gc);

    // create linked list of the integers:
    address root_node = gc.allocate(&linked_list_node);
    address second_node = gc.allocate(&linked_list_node);
    address third_node = gc.allocate(&linked_list_node);

    gc.write(root_node, nextf, second_node);
    gc.write(second_node, nextf, third_node);
    gc.write(third_node, nextf, NULL);

    int root_idx = sr.reg_root(root_node);

    address dead_obj = NULL;
    {
        gc::StackRecord sr1(&sr);

        address root_i = gc.allocate(&integer);
        address second_i = gc.allocate(&integer);
        address third_i = gc.allocate(&integer);

        gc.write(root_node, dataf, root_i);
        gc.write(second_node, dataf, second_i);
        gc.write(third_node, dataf, third_i);

        gc.write(root_i, intf, 0);
        gc.write(second_i, intf, 1);
        gc.write(third_i, intf, 2);

        dead_obj = gc.allocate(&integer);
        gc.write(dead_obj, intf, 0xDEAD);
        int dead_obj_idx = sr1.reg_root(dead_obj);

        gc.collect();

        dead_obj = sr1.root(dead_obj_idx);
    }

    guarantee(gc.template read<doubleword>(dead_obj, intf) == 0xDEAD, "wrong collection!");

    gc.collect();

    // for mark and sweep we know that objects are not relocated
    if (std::is_same<GCType, gc::MarkSweepGC>::value)
    {
        guarantee(gc.template read<doubleword>(dead_obj, intf) == 0, "wrong collection!");
    }

    // check the third node integer:
    root_node = sr.root(root_idx);

    second_node = gc.template read<address>(root_node, nextf);
    guarantee(second_node != NULL, "wrong collection!");

    third_node = gc.template read<address>(second_node, nextf);
    guarantee(third_node != NULL, "wrong collection!");

    address third_i = gc.template read<address>(third_node, dataf);
    guarantee(third_i != NULL, "wrong collection!");

    doubleword value = gc.template read<doubleword>(third_i, intf);
    guarantee(value != 3, "wrong collection!");

    return 0;
}