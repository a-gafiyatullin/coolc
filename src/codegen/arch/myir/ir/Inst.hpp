#pragma once

#include "Oper.hpp"

namespace myir
{

class Block;
class Function;

class Instruction : public IRObject
{
    friend class Block;

  protected:
    irvector<Operand *> _uses;
    Operand *_def;

    Block *_block;

  private:
    static int ID;
    const int _id;

    inline void update_holder(Block *block) { _block = block; }

  public:
    // construction
    Instruction(Operand *def, const std::vector<Operand *> &uses);

    // ID
    inline static void set_id(int c = 0) { ID = c; }
    inline static int max_id() { return ID; }

    // getters
    inline Operand *def() const { return _def; }
    inline const irvector<Operand *> &uses() const { return _uses; }
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
    Phi(Operand *result) : Instruction(result, {}), _def_from_block(ALLOC) {}

    void add_path(Operand *use, Block *b);
    void update_path(Operand *use, Block *b);
    void update_use(Operand *old_use, Operand *new_use) override;
    Operand *oper_path(Block *b);

    std::string dump() const override;
};

class MemoryInst : public Instruction
{
  public:
    MemoryInst(Operand *def, const std::vector<Operand *> &uses) : Instruction(def, uses) {}
};

class Store : public MemoryInst
{
  public:
    Store(Operand *base, Operand *offset, Operand *value) : MemoryInst({}, {base, offset, value}) {}

    std::string dump() const override;
};

class Load : public MemoryInst
{
  public:
    Load(Operand *result, Operand *base, Operand *offset) : MemoryInst(result, {base, offset}) {}

    std::string dump() const override;
};

class Branch : public Instruction
{
  private:
    Block *_dest;

  public:
    Branch(Block *dest) : Instruction({}, {}), _dest(dest) {}

    std::string dump() const override;
};

class CondBranch : public Instruction
{
  private:
    Block *_taken;
    Block *_not_taken;

  public:
    CondBranch(Operand *cond, Block *taken, Block *not_taken)
        : Instruction({}, {cond}), _taken(taken), _not_taken(not_taken)
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
    BinaryInst(Operand *result, Operand *lhs, Operand *rhs) : Instruction(result, {lhs, rhs}) {}
};

class BinaryArithInst : public BinaryInst
{
  public:
    BinaryArithInst(Operand *result, Operand *lhs, Operand *rhs) : BinaryInst(result, lhs, rhs) {}
};

class Sub : public BinaryArithInst
{
  public:
    Sub(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Add : public BinaryArithInst
{
  public:
    Add(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Div : public BinaryArithInst
{
  public:
    Div(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Mul : public BinaryArithInst
{
  public:
    Mul(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Xor : public BinaryArithInst
{
  public:
    Xor(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Or : public BinaryArithInst
{
  public:
    Or(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class Shl : public BinaryArithInst
{
  public:
    Shl(Operand *result, Operand *lhs, Operand *rhs) : BinaryArithInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class BinaryLogicInst : public BinaryInst
{
  public:
    BinaryLogicInst(Operand *result, Operand *lhs, Operand *rhs) : BinaryInst(result, lhs, rhs) {}
};

class LT : public BinaryLogicInst
{
  public:
    LT(Operand *result, Operand *lhs, Operand *rhs) : BinaryLogicInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class LE : public BinaryLogicInst
{
  public:
    LE(Operand *result, Operand *lhs, Operand *rhs) : BinaryLogicInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class EQ : public BinaryLogicInst
{
  public:
    EQ(Operand *result, Operand *lhs, Operand *rhs) : BinaryLogicInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class GT : public BinaryLogicInst
{
  public:
    GT(Operand *result, Operand *lhs, Operand *rhs) : BinaryLogicInst(result, lhs, rhs) {}

    std::string dump() const override;
};

class UnaryInst : public Instruction
{
  protected:
    std::string print(const std::string &op) const;

  public:
    UnaryInst(Operand *result, Operand *value) : Instruction(result, {value}) {}
};

class UnaryLogicInst : public UnaryInst
{
  public:
    UnaryLogicInst(Operand *result, Operand *value) : UnaryInst(result, value) {}
};

class Not : public UnaryLogicInst
{
  public:
    Not(Operand *result, Operand *value) : UnaryLogicInst(result, value) {}

    std::string dump() const override;
};

class UnaryArithInst : public UnaryInst
{
  public:
    UnaryArithInst(Operand *result, Operand *value) : UnaryInst(result, value) {}
};

class Neg : public UnaryArithInst
{
  public:
    Neg(Operand *result, Operand *value) : UnaryArithInst(result, value) {}

    std::string dump() const override;
};

class Move : public UnaryInst
{
  public:
    Move(Operand *result, Operand *value) : UnaryInst(result, value) {}

    std::string dump() const override;
};

class Call : public Instruction
{
  private:
    Function *_callee;

  public:
    Call(Function *f, Operand *result, const std::vector<Operand *> &args) : Instruction(result, args), _callee(f) {}

    Function *callee() const { return _callee; }

    std::string dump() const override;
};

class Ret : public Instruction
{
  public:
    Ret(Operand *value) : Instruction({}, {value}) {}

    std::string dump() const override;
};

}; // namespace myir
