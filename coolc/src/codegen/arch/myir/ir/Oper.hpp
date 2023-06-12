#pragma once

#include "allocator/Allocator.hpp"
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace myir
{

class Instruction;
class Block;
class Constant;
class Operand;
class Function;
class Variable;
class CFG;

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

// class represents temporary result
class Operand : public IRObject
{
    friend class Instruction;

  protected:
    const irstring _name;
    const OperandType _type;
    int _id;

    irvector<Instruction *> _uses;
    irvector<Instruction *> _defs;

    // record use-def and def-use chains during instruction construction
    inline void used_by(Instruction *inst) { _uses.push_back(inst); }
    inline void defed_by(Instruction *inst) { _defs.push_back(inst); }

    Operand(const std::string &name, OperandType type)
        : _name(name ALLOCCOMMA), _type(type), _id(INCORRECT_ID), _uses(ALLOC), _defs(ALLOC)
    {
    }

  private:
    static constexpr std::string_view PREFIX = "tmp";
    static int ID;
    static constexpr int INCORRECT_ID = -1;

  public:
    // constructors
    Operand(OperandType type) : _name(PREFIX ALLOCCOMMA), _id(ID++), _type(type), _uses(ALLOC), _defs(ALLOC) {}

    Operand(const Operand &other) : _type(other._type), _name(other._name), _id(ID++), _uses(ALLOC), _defs(ALLOC) {}

    // type
    inline OperandType type() const { return _type; }

    // typecheck
    template <class T> static bool isa(Operand *o);
    template <class T> static T *as(Operand *o);

    // array of defs
    inline irvector<Instruction *> &defs() { return _defs; }

    // debugging
    virtual std::string dump() const;
    virtual std::string name() const
    {
        return _id != INCORRECT_ID ? std::string(_name) + std::to_string(_id) : std::string(_name);
    }
};

class Constant : public Operand
{
  private:
    uint64_t _value;

  public:
    // constructors
    Constant(uint64_t value, OperandType type) : Operand("imm", type), _value(value) {}

    // type
    inline uint64_t value() const { return _value; }

    // debugging
    std::string dump() const override;
    std::string name() const override { return dump(); }
};

class Variable : public Operand
{
  private:
    Variable *_original_var; // original variable for all renamed vars

  public:
    // constructors
    Variable(const std::string &name, OperandType type) : Operand(name, type), _original_var(this) {}

    Variable(OperandType type) : Operand(type), _original_var(this) {}

    Variable(const Variable &other) : Operand(other), _original_var(other._original_var) {}

    // getters
    inline Variable *original_var() const { return _original_var; }

    // debugging
    std::string name() const override;
};

class StructuredOperand : public Operand
{
  private:
    irvector<Operand *> _fields;

  public:
    // constructors

    // if type is STRUCTURE, then use internal structure in _fields
    StructuredOperand(const std::string &name, const std::vector<Operand *> &fields, OperandType type)
        : Operand(name, type), _fields(fields.begin(), fields.end() ALLOCCOMMA)
    {
    }

    // debugging
    std::string dump() const override;
};

// global constant has address
class GlobalConstant : public StructuredOperand
{
  public:
    // constructors
    GlobalConstant(const std::string &name, const std::vector<Operand *> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }
};

// global variable has address
class GlobalVariable final : public StructuredOperand
{
  public:
    // constructors
    // if type is STRUCTURE, then use internal structure in _fields
    GlobalVariable(const std::string &name, const std::vector<Operand *> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }
};

class Function final : public GlobalConstant
{
  private:
    const irvector<Variable *> _params;

    CFG *_cfg;

    const OperandType _return_type;
    bool _is_leaf;

  public:
    // constructors
    Function(const std::string &name, const std::vector<Variable *> &params, OperandType return_type);

    // CFG
    void set_cfg(Block *cfg);
    inline CFG *cfg() const { return _cfg; }

    // params
    inline Variable *param(int i) const { return _params.at(i); }
    inline int params_size() const { return _params.size(); }

    // return
    inline OperandType return_type() const { return _return_type; }
    inline bool has_return() const { return return_type() != VOID; }

    // type
    inline void set_is_leaf() { _is_leaf = true; }

    // debugging
    std::string name() const override;
    std::string short_name() const;
    std::string dump() const override;
};

}; // namespace myir