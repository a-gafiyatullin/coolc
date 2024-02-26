#include "IR.hpp"
#include "utils/Utils.h"
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
    Operand *res = nullptr;

    if (Operand::isa<Constant>(lhs) && Operand::isa<Constant>(rhs))
    {
        auto lhsv = Operand::as<Constant>(lhs)->value();
        auto rhsv = Operand::as<Constant>(rhs)->value();

        uint64_t value = 0;
        if (std::is_same_v<T, Add>)
        {
            value = lhsv + rhsv;
        }
        else if (std::is_same_v<T, Sub>)
        {
            value = lhsv - rhsv;
        }
        else if (std::is_same_v<T, Div>)
        {
            value = lhsv / rhsv;
        }
        else if (std::is_same_v<T, Mul>)
        {
            value = lhsv * rhsv;
        }
        else if (std::is_same_v<T, Shl>)
        {
            value = lhsv << rhsv;
        }
        else if (std::is_same_v<T, LT>)
        {
            value = lhsv < rhsv;
        }
        else if (std::is_same_v<T, LE>)
        {
            value = lhsv <= rhsv;
        }
        else if (std::is_same_v<T, EQ>)
        {
            value = lhsv == rhsv;
        }
        else if (std::is_same_v<T, Or>)
        {
            value = lhsv | rhsv;
        }
        else if (std::is_same_v<T, Xor>)
        {
            value = lhsv ^ rhsv;
        }
        else if (std::is_same_v<T, GT>)
        {
            value = lhsv > rhsv;
        }

        res = new Constant(value, lhs->type());
    }
    else
    {
        res = new Operand(lhs->type());
        _curr_block->append(new T(res, lhs, rhs));
    }

    return res;
}

template <class T> myir::Operand *myir::IRBuilder::unary(Operand *operand)
{
    auto *res = new Operand(operand->type());
    _curr_block->append(new T(res, operand));

    return res;
}

template <myir::OperandType type> myir::Operand *myir::IRBuilder::ld(Operand *base, Operand *offset)
{
    // we can optimize loads from constant objects by constant offset
    if (Operand::isa<GlobalConstant>(base) && Operand::isa<Constant>(offset))
    {
        auto *global_constant = Operand::as<GlobalConstant>(base);
        auto *constant_offset = Operand::as<Constant>(offset);

        return global_constant->field(constant_offset->value());
    }

    myir::Operand *res = new Operand(type);
    _curr_block->append(new Load(res, base, offset));

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
    {
        assert(false && "Unexpected type");
    }
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
    {
        assert(false && "Unexpected type");
    }
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
    {
        assert(false && "Unexpected type");
    }
}

template <myir::Block::AppendType type> void myir::Block::append(Instruction *inst, Instruction *newinst)
{
    if constexpr (type == FRONT)
    {
        _insts.push_front(newinst);
        newinst->update_holder(this);
    }
    else if constexpr (type == BACK)
    {
        _insts.push_back(newinst);
        newinst->update_holder(this);
    }
    else
    {
        for (auto iter = _insts.begin(); iter != _insts.end(); iter++)
        {
            if (*iter == inst)
            {
                if constexpr (type == AFTER)
                {
                    iter++;
                }
                else if constexpr (type == INSTEAD)
                {
                    iter = erase_common(iter);
                }
                _insts.insert(iter, newinst);
                newinst->update_holder(this);
                return;
            }
        }

        SHOULD_NOT_REACH_HERE();
    }
}
