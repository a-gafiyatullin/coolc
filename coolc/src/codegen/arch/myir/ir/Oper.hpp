#pragma once

#include "allocator/Allocator.hpp"

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

    // language primitives
    INTEGER,
    BOOLEAN,
    STRING,

    STRUCTURE,
    VOID,

    OperandTypeSize
};

// Class represents a temporary result
class Operand : public IRObject
{
    friend class Instruction;
    friend class Phi;

  protected:
    const irstring _name;
    OperandType _type;
    const int _id;

    irvector<Instruction *> _uses;
    irvector<Instruction *> _defs;

    Operand(const std::string &name, OperandType type) : Operand(name, type, ID++) {}

    Operand(const std::string &name, OperandType type, int id)
        : _name(name ALLOCCOMMA), _type(type), _id(id), _uses(ALLOC), _defs(ALLOC)
    {
    }

    // operand can have more than one def before SSA construction
    inline irvector<Instruction *> &defs() { return _defs; }

    // record use-def and def-use chains during instruction construction
    inline void used_by(Instruction *inst) { _uses.push_back(inst); }
    inline void defined_by(Instruction *inst) { _defs.push_back(inst); }

  private:
    static constexpr std::string_view PREFIX = "tmp";
    static int ID;

  public:
    // constructors
    Operand(OperandType type) : Operand((std::string)PREFIX, type) {}

    Operand(const Operand &other) : Operand((std::string)other._name, other._type) {}

    // id
    inline static void set_id(int c = 0) { ID = c; }
    inline static int max_id() { return ID; }

    // getters
    inline OperandType type() const { return _type; }
    inline void set_type(OperandType type) { _type = type; }
    inline int id() const { return _id; }

    // uses and def
    inline const irvector<Instruction *> &uses() const { return _uses; }
    inline Instruction *use(int i) const { return _uses.at(i); }
    inline void erase_use(Instruction *use) { _uses.erase(std::find(_uses.begin(), _uses.end(), use)); }

    inline bool has_def() const { return !_defs.empty(); }
    inline Instruction *def() const { return _defs.at(0); }
    void erase_def(Instruction *def);

    // typecheck
    template <class T> static bool isa(Operand *o);
    template <class T> static T *as(Operand *o);

    // debugging
    virtual std::string dump() const;
    virtual std::string name() const { return std::string(_name) + std::to_string(_id); }
};

class Constant : public Operand
{
  private:
    const uint64_t _value;

  public:
    // constructors
    Constant(uint64_t value, OperandType type) : Operand("imm", type), _value(value) {}

    // type
    inline uint64_t value() const { return _value; }

    // debugging
    std::string dump() const override { return name(); }
    std::string name() const override;
};

class Variable : public Operand
{
  private:
    Variable *const _original_var; // original variable for all renamed vars

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
    const irvector<Operand *> _fields;

  public:
    // constructors

    // if type is STRUCTURE, then use internal structure in _fields
    StructuredOperand(const std::string &name, const std::vector<Operand *> &fields, OperandType type)
        : Operand(name, type, -1), _fields(fields.begin(), fields.end() ALLOCCOMMA)
    {
    }

    // get field by offset
    Operand *field(int offset) const;
    Operand *word(int offset) const;

    // debugging
    std::string name() const override { return (std::string)_name; }
    std::string dump() const override;
};

// Global constant has address
class GlobalConstant : public StructuredOperand
{
  public:
    // constructors
    GlobalConstant(const std::string &name, const std::vector<Operand *> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }
};

// Global variable has address
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
    irvector<Variable *> _params;
    const OperandType _return_type;

    CFG *_cfg;

    // kind of method
    union Kind {
        int _is_leaf : 1;
        int _is_init : 1;
        int _is_runtime : 1;
    } _kind;

    // IDs
    int _max_operand_id;
    int _max_instruction_id;
    int _max_block_id;

  public:
    // constructors
    Function(const std::string &name, const std::vector<Variable *> &params, OperandType return_type);

    // CFG
    void set_cfg(Block *cfg);
    inline CFG *cfg() const { return _cfg; }

    // params
    inline Variable *param(int i) const { return _params.at(i); }
    inline void set_param(int i, Variable *var) { _params[i] = var; }
    inline int params_size() const { return _params.size(); }
    const irvector<Variable *> &params() const { return _params; }

    // return
    inline OperandType return_type() const { return _return_type; }
    inline bool has_return() const { return return_type() != VOID; }

    // type
    inline void set_is_leaf() { _kind._is_leaf = true; }
    inline void set_is_init() { _kind._is_init = true; }
    inline void set_is_runtime() { _kind._is_runtime = true; }
    inline bool is_leaf() const { return _kind._is_leaf; }
    inline bool is_init() const { return _kind._is_init; }
    inline bool is_runtime() const { return _kind._is_runtime; }

    // debugging
    void record_max_ids();
    void reset_max_ids();

    std::string name() const override;
    std::string short_name() const;
    std::string dump() const override;
};

}; // namespace myir