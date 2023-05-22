#pragma once

#include "Inst.hpp"
#include <list>

namespace myir
{

class Block
{
    friend class GraphUtils;

  private:
    const std::string _name;

    std::vector<block> _preds;
    std::vector<block> _succs;

    std::list<inst> _insts;

    bool _is_visited;

    int _postorder_num;

  public:
    Block(const std::string &name) : _name(name), _is_visited(false), _postorder_num(-1)
    {
    }

    inline void append(const inst &inst)
    {
        _insts.push_back(inst);
    }

    static void connect(const block &pred, const block &succ);

    const std::string &name() const
    {
        return _name;
    }

    inline int postorder() const
    {
        assert(_postorder_num != -1);
        return _postorder_num;
    }

    inline std::list<inst> &insts()
    {
        return _insts;
    }

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
    template <class T> void add(const T &elem)
    {
        if constexpr (std::is_same_v<T, func>)
        {
            _funcs[elem->short_name()] = elem;
        }
        else if constexpr (std::is_same_v<T, global_const>)
        {
            _constants[elem->name()] = elem;
        }
        else if constexpr (std::is_same_v<T, global_var>)
        {
            _variables[elem->name()] = elem;
        }
        else
            static_assert("Unexpected type");
    }

    template <class T> T get(const std::string &name) const
    {
        if constexpr (std::is_same_v<T, func>)
        {
            return get_by_name(name, _funcs);
        }
        else if constexpr (std::is_same_v<T, global_const>)
        {
            return get_by_name(name, _constants);
        }
        else if constexpr (std::is_same_v<T, global_var>)
        {
            return get_by_name(name, _variables);
        }
        else
            static_assert("Unexpected type");
    }

    template <class T> const std::unordered_map<std::string, T> &get() const
    {
        if constexpr (std::is_same_v<T, func>)
        {
            return _funcs;
        }
        else if constexpr (std::is_same_v<T, global_const>)
        {
            return _constants;
        }
        else if constexpr (std::is_same_v<T, global_var>)
        {
            return _variables;
        }
        else
            static_assert("Unexpected type");
    }

    // Transform all functions' CFG to SSA form
    void construct_ssa();

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
    static inst phi(const oper& var);

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