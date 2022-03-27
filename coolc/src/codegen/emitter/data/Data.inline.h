// TODO: maybe pragma once?
#include "Data.h"

using namespace codegen;

template <class Value, class ClassDesc, class DispTableDesc>
Data<Value, ClassDesc, DispTableDesc>::Data(const std::shared_ptr<KlassBuilder> &builder) : _builder(builder)
{
    _builder->init();
}

template <class Value, class ClassDesc, class DispTableDesc>
Value Data<Value, ClassDesc, DispTableDesc>::init_value(const std::shared_ptr<ast::Type> &type)
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

template <class Value, class ClassDesc, class DispTableDesc>
Value Data<Value, ClassDesc, DispTableDesc>::string_const(const std::string &str)
{
    if (_string_constants.find(str) == _string_constants.end())
    {
        string_const_inner(str);
    }

    return _string_constants.at(str);
}

template <class Value, class ClassDesc, class DispTableDesc>
Value Data<Value, ClassDesc, DispTableDesc>::bool_const(const bool &value)
{
    if (_bool_constants.find(value) == _bool_constants.end())
    {
        bool_const_inner(value);
    }

    return _bool_constants.at(value);
}

template <class Value, class ClassDesc, class DispTableDesc>
Value Data<Value, ClassDesc, DispTableDesc>::int_const(const int64_t &value)
{
    if (_int_constants.find(value) == _int_constants.end())
    {
        int_const_inner(value);
    }

    return _int_constants.at(value);
}

template <class Value, class ClassDesc, class DispTableDesc>
ClassDesc Data<Value, ClassDesc, DispTableDesc>::class_struct(const std::shared_ptr<Klass> &klass)
{
    if (_classes.find(klass->name()) == _classes.end())
    {
        class_struct_inner(klass);
    }

    return _classes.at(klass->name());
}

template <class Value, class ClassDesc, class DispTableDesc>
DispTableDesc Data<Value, ClassDesc, DispTableDesc>::class_disp_tab(const std::shared_ptr<Klass> &klass)
{
    if (_dispatch_tables.find(klass->name()) == _dispatch_tables.end())
    {
        class_disp_tab_inner(klass);
    }

    return _dispatch_tables.at(klass->name());
}

template <class Value, class ClassDesc, class DispTableDesc>
void Data<Value, ClassDesc, DispTableDesc>::emit(std::ofstream &out_file)
{
    int_const(0);
    string_const("");

    gen_class_obj_tab();
    gen_class_name_tab();

    emit_inner(out_file);
}