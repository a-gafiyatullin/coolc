#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
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

typedef std::shared_ptr<Instruction> inst;
typedef std::shared_ptr<Block> block;
typedef std::shared_ptr<Operand> oper;
typedef std::shared_ptr<Constant> constant;
typedef std::shared_ptr<Variable> variable;
typedef std::shared_ptr<Function> func;

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
class Operand
{
    friend class IRBuilder;

  protected:
    const std::string _name;
    const OperandType _type;
    int _id;

    std::vector<inst> _uses;
    std::vector<inst> _defs;

    // record use-def and def-use chains during instruction construction
    inline void used_by(const inst &inst) { _uses.push_back(inst); }
    inline void defed_by(const inst &inst) { _defs.push_back(inst); }

    Operand(const std::string &name, OperandType type) : _name(name), _type(type), _id(INCORRECT_ID) {}

  private:
    static constexpr std::string_view PREFIX = "tmp";
    static int ID;
    static constexpr int INCORRECT_ID = -1;

  public:
    // constructors
    Operand(OperandType type) : _name(PREFIX), _id(ID++), _type(type) {}

    Operand(const Operand &other) : _type(other._type), _name(other._name), _id(ID++) {}

    // convinient construction
    static inline oper operand(OperandType type) { return std::make_shared<Operand>(type); }

    // type
    inline OperandType type() const { return _type; }

    // typecheck
    template <class T> static bool isa(const oper &o);
    template <class T> static std::shared_ptr<T> as(const oper &o);

    // array of defs
    inline std::vector<inst> &defs() { return _defs; }

    // debugging
    virtual std::string dump() const;
    virtual std::string name() const { return _id != INCORRECT_ID ? _name + std::to_string(_id) : _name; }
};

class Constant : public Operand
{
  private:
    uint64_t _value;

  public:
    // constructors
    Constant(uint64_t value, OperandType type) : Operand("imm", type), _value(value) {}

    // convinient construction
    static inline constant constval(OperandType type, int value) { return std::make_shared<Constant>(value, type); }

    // type
    inline uint64_t value() const { return _value; }

    // debugging
    std::string dump() const override;
    std::string name() const override { return dump(); }
};

class Variable : public Operand
{
  private:
    oper _reaching_def;

  public:
    // constructors
    Variable(const std::string &name, OperandType type) : Operand(name, type), _reaching_def(nullptr) {}

    // convinient construction
    static inline variable var(const std::string &name, OperandType type)
    {
        return std::make_shared<Variable>(name, type);
    }

    static inline variable var(const variable &var) { return std::make_shared<Variable>(*var); }
};

class StructuredOperand : public Operand
{
  private:
    std::vector<oper> _fields;

  public:
    // constructors

    // if type is STRUCTURE, then use internal structure in _fields
    StructuredOperand(const std::string &name, const std::vector<oper> &fields, OperandType type)
        : Operand(name, type), _fields(fields)
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
    GlobalConstant(const std::string &name, const std::vector<oper> &fields, OperandType type)
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
    GlobalVariable(const std::string &name, const std::vector<oper> &fields, OperandType type)
        : StructuredOperand(name, fields, type)
    {
    }
};

class Function final : public GlobalConstant
{
  private:
    const std::vector<oper> _params;
    const OperandType _return_type;

    const std::shared_ptr<CFG> _cfg;

    bool _is_leaf;

    // for every variable with multiple defs find all blocks that contain it
    std::unordered_map<oper, std::set<block>> defs_in_blocks() const;

    // standard algorithm for iinserting phi-functions
    void insert_phis(const std::unordered_map<oper, std::set<block>> &vars_in_blocks,
                     const std::unordered_map<block, std::set<block>> &df);

#ifdef DEBUG
    static void dump_defs(const std::unordered_map<oper, std::set<block>> &defs);
#endif // DEBUG

  public:
    // constructors
    Function(const std::string &name, const std::vector<oper> &params, OperandType return_type);

    // CFG
    void set_cfg(const block &cfg);
    inline std::shared_ptr<CFG> cfg() const { return _cfg; }

    // Transform CFG to SSA form
    void construct_ssa();

    // params
    inline oper param(int i) const { return _params.at(i); }
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