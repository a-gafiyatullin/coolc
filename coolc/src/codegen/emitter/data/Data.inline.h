// TODO: maybe pragma once?
#include "Data.h"

using namespace codegen;

template <class Value, class ClassDesc>
Data<Value, ClassDesc>::Data(const std::shared_ptr<KlassBuilder> &builder) : _builder(builder)
{
    _builder->init();
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::init_value(const std::shared_ptr<ast::Type> &type)
{
    if (semant::Semant::is_string(type))
    {
        return string_const("");
    }
    if (semant::Semant::is_int(type))
    {
        return int_const(0);
    }
    if (semant::Semant::is_bool(type))
    {
        return bool_const(false);
    }

    SHOULD_NOT_REACH_HERE();
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::string_const(const std::string &str)
{
    // TODO: need log here
    if (_string_constants.find(str) == _string_constants.end())
    {
        string_const_inner(str);
    }

    return _string_constants.at(str);
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::bool_const(const bool &value)
{
    // TODO: need log here
    if (_bool_constants.find(value) == _bool_constants.end())
    {
        bool_const_inner(value);
    }

    return _bool_constants.at(value);
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::int_const(const int64_t &value)
{
    // TODO: need log here
    if (_int_constants.find(value) == _int_constants.end())
    {
        int_const_inner(value);
    }

    return _int_constants.at(value);
}

template <class Value, class ClassDesc>
ClassDesc Data<Value, ClassDesc>::class_struct(const std::shared_ptr<Klass> &klass)
{
    // TODO: need log here
    if (_classes.find(klass->name()) == _classes.end())
    {
        class_struct_inner(klass);
    }

    return _classes.at(klass->name());
}

template <class Value, class ClassDesc>
Value Data<Value, ClassDesc>::class_disp_tab(const std::shared_ptr<Klass> &klass)
{
    // TODO: need log here
    if (_dispatch_tables.find(klass->name()) == _dispatch_tables.end())
    {
        class_disp_tab_inner(klass);
    }

    return _dispatch_tables.at(klass->name());
}

template <class Value, class ClassDesc> void Data<Value, ClassDesc>::emit(const std::string &out_file)
{
    // TODO: do we reaaly need this here?
    int_const(0);
    string_const("");

    gen_class_obj_tab();
    gen_class_name_tab();

    emit_inner(out_file);
}