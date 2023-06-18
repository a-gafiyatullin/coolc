#pragma once

#include "Inst.hpp"
#include <list>

namespace myir
{

class Block : public IRObject
{
    friend class CFG;

  private:
    const irstring _name;

    const int _id;
    int _postorder_num;

    Function *_func;

    irvector<Block *> _preds;
    irvector<Block *> _succs;

    irlist<Instruction *> _insts;

    static int ID;

  public:
    // construction
    Block(const std::string &name, Function *f)
        : _name(name ALLOCCOMMA), _postorder_num(-1), _func(f), _preds(ALLOC), _succs(ALLOC), _insts(ALLOC), _id(ID++)
    {
    }

    // ID
    static void reset_id() { ID = 0; }
    static int max_id() { return ID; }

    // add instruction
    inline void append(Instruction *inst) { _insts.push_back(inst); }
    inline void append_front(Instruction *inst) { _insts.push_front(inst); }

    // create edges in CFG
    static void connect(Block *pred, Block *succ);

    // number that was set by postorder traversal
    inline int postorder() const { return _postorder_num; }

    // getters
    inline irlist<Instruction *> &insts() { return _insts; }
    inline Function *holder() const { return _func; }
    inline const irvector<Block *> &succs() const { return _succs; }
    inline int id() const { return _id; }

    // debugging
    std::string name() const { return std::string(_name); }
    std::string dump() const;
};

class Module : public allocator::StackObject
{
  private:
    // text segment
    std::unordered_map<std::string, Function *> _funcs;

    // rodata segment
    std::unordered_map<std::string, GlobalConstant *> _constants;

    // data segment
    std::unordered_map<std::string, GlobalVariable *> _variables;

    // convinient access to arrays
    template <class T> T get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const;

  public:
    // add and get global variables and constants
    template <class T> void add(T elem);
    template <class T> T *get(const std::string &name) const;

    // raw access to variables and constants
    template <class T> const std::unordered_map<std::string, T *> &get() const;

    // debugging
    std::string dump() const;
};

// track the current state of the CFG construction
class IRBuilder : public allocator::StackObject
{
  private:
    Module &_module;

    Block *_curr_block;

    Function *_curr_func;

    // convinience methods for bnary and unary instructions
    template <class T> Operand *binary(Operand *lhs, Operand *rhs);
    template <class T> Operand *unary(Operand *operand);

  public:
    // construction
    IRBuilder(Module &module) : _module(module), _curr_block(nullptr) {}

    // reset
    static void reset();

    // create a new block
    inline Block *new_block(const std::string &name) { return new Block(name, _curr_func); }

    // set a state
    inline void set_current_function(Function *func) { _curr_func = func; }
    inline void set_current_block(Block *block) { _curr_block = block; }

    // get a state
    inline Block *curr_block() const { return _curr_block; }

    // create instructions
    static void phi(Operand *var, Block *b);

    void ret(Operand *value);

    void st(Operand *base, Operand *offset, Operand *value);

    template <OperandType type> Operand *ld(Operand *base, Operand *offset);

    Operand *call(Function *f, const std::vector<Operand *> &args);

    Operand *call(Function *f, Operand *dst, const std::vector<Operand *> &args);

    void cond_br(Operand *pred, Block *taken, Block *fall_through);

    void br(Block *taken);

    Operand *add(Operand *lhs, Operand *rhs);

    Operand *sub(Operand *lhs, Operand *rhs);

    Operand *div(Operand *lhs, Operand *rhs);

    Operand *mul(Operand *lhs, Operand *rhs);

    Operand *shl(Operand *lhs, Operand *rhs);

    Operand *lt(Operand *lhs, Operand *rhs);

    Operand *le(Operand *lhs, Operand *rhs);

    Operand *eq(Operand *lhs, Operand *rhs);

    Operand *gt(Operand *lhs, Operand *rhs);

    Operand *or2(Operand *lhs, Operand *rhs);

    Operand *xor2(Operand *lhs, Operand *rhs);

    Operand *neg(Operand *operand);

    Operand *not1(Operand *operand);

    Operand *move(Operand *src);

    void move(Operand *src, Operand *dst);
};
} // namespace myir