#include "IR.hpp"

using namespace myir;

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