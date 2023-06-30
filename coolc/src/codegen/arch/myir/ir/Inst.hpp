#pragma once

#include "Oper.hpp"

namespace myir
{

class Block;
class Function;

class Instruction : public IRObject
{
  protected:
    irvector<Operand *> _uses;
    Operand *_def;

    Block *_block;

  private:
    static int ID;
    const int _id;

  public:
    // construction
    Instruction(Operand *def, const std::vector<Operand *> &uses, Block *b);

    // ID
    inline static void set_id(int c = 0) { ID = c; }
    inline static int max_id() { return ID; }

    // getters
    inline Operand *def() { return _def; }
    inline irvector<Operand *> &uses() { return _uses; }
    inline Operand *use(int i) const { return _uses.at(i); }
    inline int id() const { return _id; }
    inline Block *holder() const { return _block; }

    // setters
    void update_def(Operand *oper);
    virtual void update_use(Operand *old_use, Operand *new_use);

    // typecheck
    template <class T> static bool isa(Instruction *inst);
    template <class T> static T *as(Instruction *inst);

    // debugging
    virtual std::string dump() const = 0;
};

class Phi : public Instruction
{
  private:
    irunordered_map<Operand *, Block *> _def_from_block;

  public:
    Phi(Operand *result, Block *b) : Instruction(result, {}, b), _def_from_block(ALLOC) {}

    void add_path(Operand *use, Block *b);
    void update_use(Operand *old_use, Operand *new_use) override;
    Operand *oper_path(Block *b);

    std::string dump() const override;
};

class MemoryInst : public Instruction
{
  public:
    MemoryInst(Operand *def, const std::vector<Operand *> &uses, Block *b) : Instruction(def, uses, b) {}
};

class Store : public MemoryInst
{
  public:
    Store(Operand *base, Operand *offset, Operand *value, Block *b) : MemoryInst({}, {base, offset, value}, b) {}

    std::string dump() const override;
};

class Load : public MemoryInst
{
  public:
    Load(Operand *result, Operand *base, Operand *offset, Block *b) : MemoryInst(result, {base, offset}, b) {}

    std::string dump() const override;
};

class Branch : public Instruction
{
  private:
    Block *_dest;

  public:
    Branch(Block *dest, Block *b) : Instruction({}, {}, b), _dest(dest) {}

    std::string dump() const override;
};

class CondBranch : public Instruction
{
  private:
    Block *_taken;
    Block *_not_taken;

  public:
    CondBranch(Operand *cond, Block *taken, Block *not_taken, Block *b)
        : Instruction({}, {cond}, b), _taken(taken), _not_taken(not_taken)
    {
    }

    inline Block *taken() const { return _taken; }
    inline Block *not_taken() const { return _not_taken; }

    std::string dump() const override;
};

class BinaryInst : public Instruction
{
  protected:
    std::string print(const std::string &op) const;

  public:
    BinaryInst(Operand *result, Operand *lhs, Operand *rhs, Block *b) : Instruction(result, {lhs, rhs}, b) {}
};

class BinaryArithInst : public BinaryInst
{
  public:
    BinaryArithInst(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryInst(result, lhs, rhs, b) {}
};

class Sub : public BinaryArithInst
{
  public:
    Sub(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Add : public BinaryArithInst
{
  public:
    Add(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Div : public BinaryArithInst
{
  public:
    Div(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Mul : public BinaryArithInst
{
  public:
    Mul(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Xor : public BinaryArithInst
{
  public:
    Xor(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Or : public BinaryArithInst
{
  public:
    Or(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class Shl : public BinaryArithInst
{
  public:
    Shl(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryArithInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class BinaryLogicInst : public BinaryInst
{
  public:
    BinaryLogicInst(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryInst(result, lhs, rhs, b) {}
};

class LT : public BinaryLogicInst
{
  public:
    LT(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryLogicInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class LE : public BinaryLogicInst
{
  public:
    LE(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryLogicInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class EQ : public BinaryLogicInst
{
  public:
    EQ(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryLogicInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class GT : public BinaryLogicInst
{
  public:
    GT(Operand *result, Operand *lhs, Operand *rhs, Block *b) : BinaryLogicInst(result, lhs, rhs, b) {}

    std::string dump() const override;
};

class UnaryInst : public Instruction
{
  protected:
    std::string print(const std::string &op) const;

  public:
    UnaryInst(Operand *result, Operand *value, Block *b) : Instruction(result, {value}, b) {}
};

class UnaryLogicInst : public UnaryInst
{
  public:
    UnaryLogicInst(Operand *result, Operand *value, Block *b) : UnaryInst(result, value, b) {}
};

class Not : public UnaryLogicInst
{
  public:
    Not(Operand *result, Operand *value, Block *b) : UnaryLogicInst(result, value, b) {}

    std::string dump() const override;
};

class UnaryArithInst : public UnaryInst
{
  public:
    UnaryArithInst(Operand *result, Operand *value, Block *b) : UnaryInst(result, value, b) {}
};

class Neg : public UnaryArithInst
{
  public:
    Neg(Operand *result, Operand *value, Block *b) : UnaryArithInst(result, value, b) {}

    std::string dump() const override;
};

class Move : public UnaryInst
{
  public:
    Move(Operand *result, Operand *value, Block *b) : UnaryInst(result, value, b) {}

    std::string dump() const override;
};

class Call : public Instruction
{
  private:
    Function *_callee;

  public:
    Call(Function *f, Operand *result, const std::vector<Operand *> &args, Block *b)
        : Instruction(result, args, b), _callee(f)
    {
    }

    Function *callee() const { return _callee; }

    std::string dump() const override;
};

class Ret : public Instruction
{
  public:
    Ret(Operand *value, Block *b) : Instruction({}, {value}, b) {}

    std::string dump() const override;
};

}; // namespace myir