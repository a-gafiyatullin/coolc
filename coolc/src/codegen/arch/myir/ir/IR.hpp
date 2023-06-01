#pragma once

#include "Inst.hpp"
#include <list>

namespace myir
{

class Block
{
    friend class CFG;

  private:
    const std::string _name;

    std::vector<block> _preds;
    std::vector<block> _succs;

    std::list<inst> _insts;

    bool _is_visited;

    int _postorder_num;

  public:
    // construction
    Block(const std::string &name) : _name(name), _is_visited(false), _postorder_num(-1) {}

    // add instruction
    inline void append(const inst &inst) { _insts.push_back(inst); }

    // create edges in CFG
    static void connect(const block &pred, const block &succ);

    // number that was set by postorder traversal
    inline int postorder() const { return _postorder_num; }

    // access to insts
    inline std::list<inst> &insts() { return _insts; }

    // debugging
    const std::string &name() const { return _name; }
    std::string dump() const;
};

typedef std::shared_ptr<GlobalConstant> global_const;
typedef std::shared_ptr<GlobalVariable> global_var;

class Module
{
  private:
    // text segment
    std::unordered_map<std::string, func> _funcs;

    // rodata segment
    std::unordered_map<std::string, global_const> _constants;

    // data segment
    std::unordered_map<std::string, global_var> _variables;

    // convinient access to arrays
    template <class T> T get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const;

  public:
    // add and get global variables and constants
    template <class T> void add(const T &elem);
    template <class T> T get(const std::string &name) const;

    // raw access to variables and constants
    template <class T> const std::unordered_map<std::string, T> &get() const;

    // Transform all functions' CFG to SSA form
    void construct_ssa();

    std::string dump() const;
};

// track the current state of the CFG construction
class IRBuilder
{
  private:
    Module &_module;

    block _curr_block;

    func _curr_func;

    // convinience methods for bnary and unary instructions
    template <class T> oper binary(const oper &lhs, const oper &rhs);
    template <class T> oper unary(const oper &operand);

  public:
    // construction
    IRBuilder(Module &module) : _module(module), _curr_block(nullptr) {}

    // create a new block
    inline static block new_block(const std::string &name) { return std::make_shared<Block>(name); }

    // set a state
    inline void set_current_function(const func &func) { _curr_func = func; }
    inline void set_current_block(const block &block) { _curr_block = block; }

    // get a state
    inline block curr_block() const { return _curr_block; }

    // create instructions
    static inst phi(const oper &var);

    void ret(const oper &value);

    void st(const oper &base, const oper &offset, const oper &value);

    template <OperandType type> oper ld(const oper &base, const oper &offset);

    oper call(const func &f, const std::vector<oper> &args);

    oper call(const func &f, const oper &dst, const std::vector<oper> &args);

    void cond_br(const oper &pred, const block &taken, const block &fall_through);

    void br(const block &taken);

    oper add(const oper &lhs, const oper &rhs);

    oper sub(const oper &lhs, const oper &rhs);

    oper div(const oper &lhs, const oper &rhs);

    oper mul(const oper &lhs, const oper &rhs);

    oper shl(const oper &lhs, const oper &rhs);

    oper lt(const oper &lhs, const oper &rhs);

    oper le(const oper &lhs, const oper &rhs);

    oper eq(const oper &lhs, const oper &rhs);

    oper gt(const oper &lhs, const oper &rhs);

    oper or2(const oper &lhs, const oper &rhs);

    oper xor2(const oper &lhs, const oper &rhs);

    oper neg(const oper &operand);

    oper not1(const oper &operand);

    oper move(const oper &src);

    void move(const oper &src, const oper &dst);
};

} // namespace myir