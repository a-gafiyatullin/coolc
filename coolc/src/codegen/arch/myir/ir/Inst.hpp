#pragma once

#include "Oper.hpp"

namespace myir
{

class Block;
class Function;

class Instruction
{
  protected:
    std::vector<oper> _uses;
    std::vector<oper> _defs;

  public:
    Instruction(const std::vector<oper> &defs, const std::vector<oper> &uses) : _defs(defs), _uses(uses)
    {
    }

    const std::vector<oper> &defs() const
    {
        return _defs;
    }

    virtual std::string dump() const = 0;
};

class Phi : public Instruction
{
  private:
    std::unordered_map<oper, block> _def_from_block;

  public:
    Phi(const oper &result) : Instruction({result}, {})
    {
    }

    std::string dump() const;
};

class MemoryInst : public Instruction
{
  public:
    MemoryInst(const std::vector<oper> &defs, const std::vector<oper> &uses) : Instruction(defs, uses)
    {
    }
};

class Store : public MemoryInst
{
  public:
    Store(const oper &base, const oper &offset, const oper &value) : MemoryInst({}, {base, offset, value})
    {
    }

    std::string dump() const override;
};

class Load : public MemoryInst
{
  public:
    Load(const oper &result, const oper &base, const oper &offset) : MemoryInst({result}, {base, offset})
    {
    }

    std::string dump() const override;
};

class Branch : public Instruction
{
  private:
    block _dest;

  public:
    Branch(const block &dest) : Instruction({}, {}), _dest(dest)
    {
    }

    std::string dump() const override;
};

class CondBranch : public Instruction
{
  private:
    block _taken;
    block _not_taken;

  public:
    CondBranch(const oper &cond, const block &taken, const block &not_taken)
        : Instruction({}, {cond}), _taken(taken), _not_taken(not_taken)
    {
    }

    std::string dump() const override;
};

class BinaryInst : public Instruction
{
  protected:
    std::string print(const std::string &op) const;

  public:
    BinaryInst(const oper &result, const oper &lhs, const oper &rhs) : Instruction({result}, {lhs, rhs})
    {
    }
};

class BinaryArithInst : public BinaryInst
{
  public:
    BinaryArithInst(const oper &result, const oper &lhs, const oper &rhs) : BinaryInst(result, lhs, rhs)
    {
    }
};

class Sub : public BinaryArithInst
{
  public:
    Sub(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Add : public BinaryArithInst
{
  public:
    Add(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Div : public BinaryArithInst
{
  public:
    Div(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Mul : public BinaryArithInst
{
  public:
    Mul(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Xor : public BinaryArithInst
{
  public:
    Xor(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Or : public BinaryArithInst
{
  public:
    Or(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class Shl : public BinaryArithInst
{
  public:
    Shl(const oper &result, const oper &lhs, const oper &rhs) : BinaryArithInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class BinaryLogicInst : public BinaryInst
{
  public:
    BinaryLogicInst(const oper &result, const oper &lhs, const oper &rhs) : BinaryInst(result, lhs, rhs)
    {
    }
};

class LT : public BinaryLogicInst
{
  public:
    LT(const oper &result, const oper &lhs, const oper &rhs) : BinaryLogicInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class LE : public BinaryLogicInst
{
  public:
    LE(const oper &result, const oper &lhs, const oper &rhs) : BinaryLogicInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class EQ : public BinaryLogicInst
{
  public:
    EQ(const oper &result, const oper &lhs, const oper &rhs) : BinaryLogicInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class GT : public BinaryLogicInst
{
  public:
    GT(const oper &result, const oper &lhs, const oper &rhs) : BinaryLogicInst(result, lhs, rhs)
    {
    }

    std::string dump() const override;
};

class UnaryInst : public Instruction
{
  protected:
    std::string print(const std::string &op) const;

  public:
    UnaryInst(const oper &result, const oper &value) : Instruction({result}, {value})
    {
    }
};

class UnaryLogicInst : public UnaryInst
{
  public:
    UnaryLogicInst(const oper &result, const oper &value) : UnaryInst(result, value)
    {
    }
};

class Not : public UnaryLogicInst
{
  public:
    Not(const oper &result, const oper &value) : UnaryLogicInst(result, value)
    {
    }

    std::string dump() const override;
};

class UnaryArithInst : public UnaryInst
{
  public:
    UnaryArithInst(const oper &result, const oper &value) : UnaryInst(result, value)
    {
    }
};

class Neg : public UnaryArithInst
{
  public:
    Neg(const oper &result, const oper &value) : UnaryArithInst(result, value)
    {
    }

    std::string dump() const override;
};

class Call : public Instruction
{
  private:
    func _callee;

  public:
    Call(const func &f, const std::vector<oper> &args) : Instruction({}, args), _callee(f)
    {
    }

    Call(const func &f, const oper &result, const std::vector<oper> &args) : Instruction({result}, args), _callee(f)
    {
    }

    std::string dump() const override;
};

class Ret : public Instruction
{
  public:
    Ret(const oper &value) : Instruction({}, {value})
    {
    }

    Ret() : Instruction({}, {})
    {
    }

    std::string dump() const override;
};

}; // namespace myir