#pragma once

#include "Inst.hpp"
#include <unordered_map>

namespace myir
{

class Block
{
  private:
    const std::string _name;

    std::vector<block> _preds;
    std::vector<block> _succs;

    std::vector<inst> _insts;

    bool _is_visited;

    static void postorder(const block &block, std::vector<myir::block> &blocks);
    static void clear_visited(const std::vector<myir::block> &blocks);

  public:
    Block(const std::string &name) : _name(name), _is_visited(false)
    {
    }

    inline void append(const inst &inst)
    {
        _insts.push_back(inst);
    }

    static void connect(const block &pred, const block &succ);

    static std::vector<block> preorder(const block &block);

    static std::vector<block> postorder(const block &block);

    const std::string &name() const
    {
        return _name;
    }

    std::string dump() const;
};

class Module
{
  private:
    // text segment
    std::unordered_map<std::string, func> _funcs;

    // rodata segment
    std::unordered_map<std::string, std::shared_ptr<GlobalConstant>> _constants;

    // data segment
    std::unordered_map<std::string, std::shared_ptr<GlobalVariable>> _variables;

    template <class T>
    inline T get_by_name(const std::string &name, const std::unordered_map<std::string, T> &map) const
    {
        auto iter = map.find(name);
        if (iter != map.end())
        {
            return iter->second;
        }
        return nullptr;
    }

  public:
    inline void add_function(const func &func)
    {
        _funcs[func->short_name()] = func;
    }

    inline void add_global_constant(const std::shared_ptr<GlobalConstant> &global)
    {
        _constants[global->name()] = global;
    }

    inline void add_global_variable(const std::shared_ptr<GlobalVariable> &variable)
    {
        _variables[variable->name()] = variable;
    }

    inline func get_function(const std::string &name) const
    {
        return get_by_name(name, _funcs);
    }

    inline std::shared_ptr<GlobalConstant> get_constant(const std::string &name) const
    {
        return get_by_name(name, _constants);
    }

    inline std::shared_ptr<GlobalVariable> get_variable(const std::string &name) const
    {
        return get_by_name(name, _variables);
    }

    std::string dump() const;
};

class IRBuilder
{
  private:
    Module &_module;

    block _curr_block;

    func _curr_func;

    template <class T> oper binary(const oper &lhs, const oper &rhs);

    template <class T> oper unary(const oper &operand);

  public:
    IRBuilder(Module &module) : _module(module), _curr_block(nullptr)
    {
    }

    inline static block new_block(const std::string &name)
    {
        return std::make_shared<Block>(name);
    }

    inline void set_current_function(const func &func)
    {
        _curr_func = func;
    }

    inline void set_current_block(const block &block)
    {
        _curr_block = block;
    }

    inline block curr_block() const
    {
        return _curr_block;
    }

    // instructions
    void ret(const oper &value);

    void st(const oper &base, const oper &offset, const oper &value);

    template <OperandType type> oper ld(const oper &base, const oper &offset)
    {
        myir::oper res = myir::Operand::operand(type);

        auto inst = std::make_shared<Load>(res, base, offset);

        base->used_by(inst);
        offset->used_by(inst);

        res->defed_by(inst);

        _curr_block->append(inst);

        return res;
    }

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