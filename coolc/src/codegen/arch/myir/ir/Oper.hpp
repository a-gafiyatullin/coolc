#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace myir
{

class Instruction;
class Block;
class Constant;

typedef std::shared_ptr<Instruction> inst;
typedef std::shared_ptr<Block> block;

enum OperandType
{
    INT8,
    UINT8,

    INT32,
    UINT32,

    INT64,
    UINT64,

    POINTER,
    STRUCTURE,
    VOID,

    OperandTypeSize
};

// class represents intermediate result
class Operand
{
    friend class IRBuilder;

  protected:
    const std::string _name;
    const OperandType _type;

    std::vector<inst> _uses;
    std::vector<inst> _defs;

    // record use-def and def-use chains during instruction construction
    inline void used_by(const inst &inst)
    {
        _uses.push_back(inst);
    }

    inline void defed_by(const inst &inst)
    {
        _defs.push_back(inst);
    }

  private:
    static constexpr std::string_view PREFIX = "tmp";
    static uint64_t ID;

  public:
    Operand(const std::string &name, OperandType type) : _name(name), _type(type)
    {
    }

    Operand(OperandType type) : _name(static_cast<std::string>(PREFIX) + std::to_string(ID++)), _type(type)
    {
    }

    inline OperandType type() const
    {
        return _type;
    }

    virtual bool is_constant() const
    {
        return false;
    }

    virtual bool is_global() const
    {
        return false;
    }

    virtual bool is_function() const
    {
        return false;
    }

    static inline std::shared_ptr<Operand> operand(OperandType type, const std::string &name)
    {
        return std::make_shared<Operand>(name, type);
    }

    static inline std::shared_ptr<Operand> operand(OperandType type)
    {
        return std::make_shared<Operand>(type);
    }

    virtual std::string dump() const;

    virtual std::string name() const
    {
        return _name;
    }
};

typedef std::shared_ptr<Operand> oper;

class Constant : public Operand
{
  private:
    uint64_t _value;

  public:
    Constant(uint64_t value, OperandType type) : Operand("imm", type), _value(value)
    {
    }

    inline uint64_t value() const
    {
        return _value;
    }

    bool is_constant() const override
    {
        return true;
    }

    std::string dump() const override;

    std::string name() const override
    {
        return dump();
    }

    static inline std::shared_ptr<Constant> constant(OperandType type, int value)
    {
        return std::make_shared<Constant>(value, type);
    }
};

class StructuredOperand : public Operand
{
  private:
    std::vector<oper> _fields;

  public:
    // if type is STRUCTURE, then use internal structure in _fields
    StructuredOperand(const std::string &name, const std::vector<oper> &fields, OperandType type)
        : Operand(name, type), _fields(fields)
    {
    }

    std::string dump() const override;
};

// global constant has address
class GlobalConstant : public StructuredOperand
{
  public:
    GlobalConstant(const std::string &name, const std::vector<oper> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }

    bool is_constant() const override
    {
        return true;
    }

    bool is_global() const override
    {
        return true;
    }
};

// global variable has address
class GlobalVariable final : public StructuredOperand
{
  public:
    // if type is STRUCTURE, then use internal structure in _fields
    GlobalVariable(const std::string &name, const std::vector<oper> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }

    bool is_global() const override
    {
        return true;
    }
};

class Function final : public GlobalConstant
{
  private:
    const std::vector<oper> _params;
    const OperandType _return_type;

    block _cfg;

    bool _is_leaf;

  public:
    Function(const std::string &name, const std::vector<oper> &params, OperandType return_type)
        : GlobalConstant(name, {}, POINTER), _params(params), _return_type(return_type), _is_leaf(false)
    {
    }

    inline void set_cfg(const block &cfg)
    {
        _cfg = cfg;
    }

    inline block cfg() const
    {
        return _cfg;
    }

    // Transform CFG to SSA form
    void construct_ssa();

    inline oper param(int i) const
    {
        assert(i < _params.size());
        return _params.at(i);
    }

    inline int params_size() const
    {
        return _params.size();
    }

    inline OperandType return_type() const
    {
        return _return_type;
    }

    inline bool has_return() const
    {
        return return_type() != VOID;
    }

    inline void set_is_leaf()
    {
        _is_leaf = true;
    }

    bool is_function() const override
    {
        return true;
    }

    std::string name() const override;

    std::string short_name() const;

    std::string dump() const override;
};

typedef std::shared_ptr<Function> func;

}; // namespace myir