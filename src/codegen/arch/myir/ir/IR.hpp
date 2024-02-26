#pragma once

#include "Inst.hpp"
#include <unordered_map>

namespace myir
{

class Block : public IRObject
{
    friend class CFG;

  private:
    const irstring _name;
    const int _id;

    Function *_func;

    int _postorder_num;

    irvector<Block *> _preds;
    irvector<Block *> _succs;
    irlist<Instruction *> _insts;

    static int ID;

    // helpers
    enum AppendType
    {
        BEFORE,
        INSTEAD,
        AFTER,
        FRONT,
        BACK
    };

    template <AppendType type> void append(Instruction *inst, Instruction *newinst);

    irlist<Instruction *>::iterator erase_common(irlist<Instruction *>::iterator inst);

  public:
    // construction
    Block(const std::string &name, Function *f)
        : _name(name ALLOCCOMMA), _postorder_num(-1), _func(f), _preds(ALLOC), _succs(ALLOC), _insts(ALLOC), _id(ID++)
    {
    }

    // ID
    inline static void set_id(int c = 0) { ID = c; }
    inline static int max_id() { return ID; }

    // add instruction
    void append(Instruction *inst);
    void append_front(Instruction *inst);
    void append_before(Instruction *inst, Instruction *newinst);
    void append_after(Instruction *inst, Instruction *newinst);
    void append_instead(Instruction *inst, Instruction *newinst);

    // delete instruction
    void erase(Instruction *inst);
    static void erase(const std::vector<Instruction *> &insts);

    // manage edges in CFG
    static void connect(Block *pred, Block *succ);
    static void disconnect(Block *pred, Block *succ);
    void disconnect();

    // number that was set by postorder traversal
    inline int postorder() const { return _postorder_num; }

    // getters
    inline const irlist<Instruction *> &insts() const { return _insts; }
    inline void clear() { _insts.clear(); }
    inline const irvector<Block *> &succs() const { return _succs; }
    inline Block *succ(int i) const { return _succs.at(i); }
    inline const irvector<Block *> &preds() const { return _preds; }
    inline Block *pred(int i) const { return _preds.at(i); }
    inline Function *holder() const { return _func; }
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

    // convenient access to arrays
    template <class T> T get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const;

  public:
    // add and get global variables and constants
    template <class T> void add(T elem);
    template <class T> T *get(const std::string &name) const;

    // raw access to variables and constants
    template <class T> const std::unordered_map<std::string, T *> &get() const;

    // debugging
    std::string dump();
};

// track the current state of the CFG construction
class IRBuilder : public allocator::StackObject
{
  private:
    Module &_module;

    Block *_curr_block;
    Function *_curr_func;

    // convenience methods for binary and unary instructions
    template <class T> Operand *binary(Operand *lhs, Operand *rhs);
    template <class T> Operand *unary(Operand *operand);

  public:
    // construction
    IRBuilder(Module &module) : _module(module), _curr_block(nullptr), _curr_func(nullptr) {}

    // create a new block
    inline Block *new_block(const std::string &name) { return new Block(name, _curr_func); }

    // setters
    void set_current_function(Function *func);
    inline void set_current_block(Block *block) { _curr_block = block; }

    // getters
    inline Block *curr_block() const { return _curr_block; }

    // helpers
    inline myir::Operand *field_offset(uint64_t offset) { return new myir::Constant(offset, myir::UINT64); }

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
