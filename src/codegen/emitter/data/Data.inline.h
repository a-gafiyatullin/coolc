#pragma once
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
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("CREATE STRING CONST \"" + str + "\""));

    if (_string_constants.find(str) == _string_constants.end())
    {
        string_const_inner(str);
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("CREATE STRING CONST \"" + str + "\""));
    return _string_constants.at(str);
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::bool_const(const bool &value)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("CREATE BOOL CONST \"" + std::to_string(value) + "\""));

    if (_bool_constants.find(value) == _bool_constants.end())
    {
        bool_const_inner(value);
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("CREATE BOOL CONST \"" + std::to_string(value) + "\""));
    return _bool_constants.at(value);
}

template <class Value, class ClassDesc> Value Data<Value, ClassDesc>::int_const(const int64_t &value)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("CREATE INT CONST \"" + std::to_string(value) + "\""));

    if (_int_constants.find(value) == _int_constants.end())
    {
        int_const_inner(value);
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("CREATE INT CONST \"" + std::to_string(value) + "\""));
    return _int_constants.at(value);
}

template <class Value, class ClassDesc>
ClassDesc Data<Value, ClassDesc>::class_struct(const std::shared_ptr<Klass> &klass)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("CREATE STRUCT FOR \"" + klass->name() + "\""));

    if (_classes.find(klass->name()) == _classes.end())
    {
        class_struct_inner(klass);
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("CREATE STRUCT FOR \"" + klass->name() + "\""));
    return _classes.at(klass->name());
}

template <class Value, class ClassDesc>
Value Data<Value, ClassDesc>::class_disp_tab(const std::shared_ptr<Klass> &klass)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("CREATE DISPATCH TABLE FOR \"" + klass->name() + "\""));

    if (_dispatch_tables.find(klass->name()) == _dispatch_tables.end())
    {
        class_disp_tab_inner(klass);
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("CREATE DISPATCH TABLE FOR \"" + klass->name() + "\""));
    return _dispatch_tables.at(klass->name());
}

template <class Value, class ClassDesc> void Data<Value, ClassDesc>::emit(const std::string &out_file)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GENERATE RUNTIME TABLES."));

    gen_class_obj_tab();
    gen_class_name_tab();

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GENERATE RUNTIME TABLES."));

    emit_inner(out_file);
}