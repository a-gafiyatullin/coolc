#include "IR.hpp"

using namespace myir;

template <class T> bool Operand::isa(const oper &o) { return std::dynamic_pointer_cast<T>(o) != nullptr; }

template <class T> std::shared_ptr<T> Operand::as(const oper &o) { return std::dynamic_pointer_cast<T>(o); }

template <class T> bool Instruction::isa(const inst &instruction)
{
    return std::dynamic_pointer_cast<T>(instruction) != nullptr;
}

template <class T> std::shared_ptr<T> Instruction::as(const inst &instruction)
{
    return std::dynamic_pointer_cast<T>(instruction);
}

template <class T> oper IRBuilder::binary(const oper &lhs, const oper &rhs)
{
    auto res = Operand::operand(lhs->type());

    auto inst = std::make_shared<T>(res, lhs, rhs);

    lhs->used_by(inst);
    rhs->used_by(inst);

    res->defed_by(inst);

    _curr_block->append(inst);

    return res;
}

template <class T> oper IRBuilder::unary(const oper &operand)
{
    auto res = Operand::operand(operand->type());

    auto inst = std::make_shared<T>(res, operand);

    operand->used_by(inst);
    res->defed_by(inst);

    _curr_block->append(inst);

    return res;
}

template <OperandType type> oper IRBuilder::ld(const oper &base, const oper &offset)
{
    myir::oper res = myir::Operand::operand(type);

    auto inst = std::make_shared<Load>(res, base, offset);

    base->used_by(inst);
    offset->used_by(inst);

    res->defed_by(inst);

    _curr_block->append(inst);

    return res;
}

template <class T> T Module::get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const
{
    auto iter = map.find(name);
    if (iter != map.end())
    {
        return iter->second;
    }
    return nullptr;
}

template <class T> void Module::add(const T &elem)
{
    if constexpr (std::is_same_v<T, func>)
    {
        _funcs[elem->short_name()] = elem;
    }
    else if constexpr (std::is_same_v<T, global_const>)
    {
        _constants[elem->name()] = elem;
    }
    else if constexpr (std::is_same_v<T, global_var>)
    {
        _variables[elem->name()] = elem;
    }
    else
        static_assert("Unexpected type");
}

template <class T> T Module::get(const std::string &name) const
{
    if constexpr (std::is_same_v<T, func>)
    {
        return get_by_name(name, _funcs);
    }
    else if constexpr (std::is_same_v<T, global_const>)
    {
        return get_by_name(name, _constants);
    }
    else if constexpr (std::is_same_v<T, global_var>)
    {
        return get_by_name(name, _variables);
    }
    else
        static_assert("Unexpected type");
}

template <class T> const std::unordered_map<std::string, T> &Module::get() const
{
    if constexpr (std::is_same_v<T, func>)
    {
        return _funcs;
    }
    else if constexpr (std::is_same_v<T, global_const>)
    {
        return _constants;
    }
    else if constexpr (std::is_same_v<T, global_var>)
    {
        return _variables;
    }
    else
        static_assert("Unexpected type");
}