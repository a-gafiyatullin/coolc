#include "IR.hpp"
#include <cassert>

template <class T> bool myir::Operand::isa(Operand *o) { return dynamic_cast<T *>(o); }

template <class T> T *myir::Operand::as(Operand *o)
{
    assert(isa<T>(o));
    return dynamic_cast<T *>(o);
}

template <class T> bool myir::Instruction::isa(Instruction *inst) { return dynamic_cast<T *>(inst); }

template <class T> T *myir::Instruction::as(Instruction *inst)
{
    assert(isa<T>(inst));
    return dynamic_cast<T *>(inst);
}

template <class T> myir::Operand *myir::IRBuilder::binary(Operand *lhs, Operand *rhs)
{
    auto *res = new Operand(lhs->type());
    _curr_block->append(new T(res, lhs, rhs, _curr_block));

    return res;
}

template <class T> myir::Operand *myir::IRBuilder::unary(Operand *operand)
{
    auto *res = new Operand(operand->type());
    _curr_block->append(new T(res, operand, _curr_block));

    return res;
}

template <myir::OperandType type> myir::Operand *myir::IRBuilder::ld(Operand *base, Operand *offset)
{
    myir::Operand *res = new Operand(type);
    _curr_block->append(new Load(res, base, offset, _curr_block));

    return res;
}

template <class T>
T myir::Module::get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const
{
    auto iter = map.find(name);
    if (iter != map.end())
    {
        return iter->second;
    }
    return nullptr;
}

template <class T> void myir::Module::add(T elem)
{
    assert(elem);
    if constexpr (std::is_same_v<T, Function *>)
    {
        _funcs[elem->short_name()] = elem;
    }
    else if constexpr (std::is_same_v<T, GlobalConstant *>)
    {
        _constants[elem->name()] = elem;
    }
    else if constexpr (std::is_same_v<T, GlobalVariable *>)
    {
        _variables[elem->name()] = elem;
    }
    else
        static_assert("Unexpected type");
}

template <class T> T *myir::Module::get(const std::string &name) const
{
    if constexpr (std::is_same_v<T, Function>)
    {
        return get_by_name(name, _funcs);
    }
    else if constexpr (std::is_same_v<T, GlobalConstant>)
    {
        return get_by_name(name, _constants);
    }
    else if constexpr (std::is_same_v<T, GlobalVariable>)
    {
        return get_by_name(name, _variables);
    }
    else
        static_assert("Unexpected type");
}

template <class T> const std::unordered_map<std::string, T *> &myir::Module::get() const
{
    if constexpr (std::is_same_v<T, Function>)
    {
        return _funcs;
    }
    else if constexpr (std::is_same_v<T, GlobalConstant>)
    {
        return _constants;
    }
    else if constexpr (std::is_same_v<T, GlobalVariable>)
    {
        return _variables;
    }
    else
        static_assert("Unexpected type");
}