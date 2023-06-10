#include "IR.inline.hpp"
#include "codegen/arch/myir/ir/CFG.hpp"
#include <cassert>

using namespace myir;

int Operand::ID = 0;

Function::Function(const std::string &name, const std::vector<Variable *> &params, OperandType return_type)
    : GlobalConstant(name, {}, POINTER), _params(params.begin(), params.end() ALLOC1COMMA), _return_type(return_type),
      _is_leaf(false), _cfg(new CFG())
{
}

std::string Block::dump() const
{
    std::string s = "  Block \"" + name() + "\", preds = [";

    for (auto *p : _preds)
    {
        s += p->_name + ", ";
    }

    trim(s, ", ");

    s += "], succs = [";

    for (auto *p : _succs)
    {
        s += p->_name + ", ";
    }

    trim(s, ", ");

    s += "]:\n";

    for (auto *i : _insts)
    {
        s += "    " + i->dump() + "\n";
    }

    trim(s, "\n");

    return s;
}

void Block::connect(Block *pred, Block *succ)
{
    assert(find(pred->_succs.begin(), pred->_succs.end(), succ) == pred->_succs.end());
    pred->_succs.push_back(succ);

    assert(find(succ->_preds.begin(), succ->_preds.end(), pred) == succ->_preds.end());
    succ->_preds.push_back(pred);
}

std::string type_to_string(OperandType type)
{
    static std::string OPERAND_TYPE_NAME[] = {"int8",   "uint8", "int32",     "uint32", "int64",
                                              "uint64", "void*", "structure", "void"};
    return OPERAND_TYPE_NAME[type];
}

std::string Function::name() const
{
    std::string s = type_to_string(_return_type) + " " + short_name() + "(";

    for (auto *p : _params)
    {
        s += p->dump() + ", ";
    }

    trim(s, ", ");

    return s + ")";
}

std::string Function::short_name() const { return std::string(_name); }

std::string Function::dump() const
{
    if (_cfg->empty())
    {
        return name() + " {}";
    }

    std::string s = name() + " {\n";

    for (auto *b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        s += b->dump() + "\n\n";
    }

    trim(s, "\n\n");

    return s + "\n}";
}

std::string Module::dump() const
{
    std::string s;

    for (auto g : _constants)
    {
        s += g.second->dump() + "\n\n";
    }

    for (auto g : _variables)
    {
        s += g.second->dump() + "\n\n";
    }

    for (auto f : _funcs)
    {
        s += f.second->dump() + "\n\n";
    }

    return s;
}

std::string Operand::dump() const { return type_to_string(_type) + " " + name(); }

std::string Constant::dump() const { return std::to_string(_value); }

std::string Variable::name() const
{
    return Operand::name() + (this != _original_var ? "[" + _original_var->name() + "]" : "");
}

std::string StructuredOperand::dump() const
{
    std::string s = "{";

    for (auto *f : _fields)
    {
        s += f->name() + ", ";
    }

    trim(s, ", ");

    s += "} " + _name;

    return s;
}

void IRBuilder::ret(Operand *value) { _curr_block->append(new Ret(value, _curr_block)); }

void IRBuilder::phi(Operand *var, Block *b) { b->append_front(new Phi(var, b)); }

void IRBuilder::st(Operand *base, Operand *offset, Operand *value)
{
    _curr_block->append(new Store(base, offset, value, _curr_block));
}

Operand *IRBuilder::call(Function *f, const std::vector<Operand *> &args) { return call(f, f, args); }

Operand *IRBuilder::call(Function *f, Operand *dst, const std::vector<Operand *> &args)
{
    std::vector<Operand *> uses;
    uses.push_back(dst);
    uses.insert(uses.end(), args.begin(), args.end());

    Operand *retval = nullptr;

    if (f->has_return())
    {
        retval = new Operand(f->return_type());
    }

    _curr_block->append(new Call(f, retval, uses, _curr_block));
    return retval;
}

void IRBuilder::cond_br(Operand *pred, Block *taken, Block *fall_through)
{
    _curr_block->append(new CondBranch(pred, taken, fall_through, _curr_block));

    Block::connect(_curr_block, taken);
    Block::connect(_curr_block, fall_through);
}

void IRBuilder::br(Block *taken)
{
    _curr_block->append(new Branch(taken, _curr_block));

    Block::connect(_curr_block, taken);
}

Operand *IRBuilder::add(Operand *lhs, Operand *rhs) { return binary<Add>(lhs, rhs); }

Operand *IRBuilder::sub(Operand *lhs, Operand *rhs) { return binary<Sub>(lhs, rhs); }

Operand *IRBuilder::div(Operand *lhs, Operand *rhs) { return binary<Div>(lhs, rhs); }

Operand *IRBuilder::mul(Operand *lhs, Operand *rhs) { return binary<Mul>(lhs, rhs); }

Operand *IRBuilder::shl(Operand *lhs, Operand *rhs) { return binary<Shl>(lhs, rhs); }

Operand *IRBuilder::lt(Operand *lhs, Operand *rhs) { return binary<LT>(lhs, rhs); }

Operand *IRBuilder::le(Operand *lhs, Operand *rhs) { return binary<LE>(lhs, rhs); }

Operand *IRBuilder::eq(Operand *lhs, Operand *rhs) { return binary<EQ>(lhs, rhs); }

Operand *IRBuilder::gt(Operand *lhs, Operand *rhs) { return binary<GT>(lhs, rhs); }

Operand *IRBuilder::or2(Operand *lhs, Operand *rhs) { return binary<Or>(lhs, rhs); }

Operand *IRBuilder::xor2(Operand *lhs, Operand *rhs) { return binary<Xor>(lhs, rhs); }

Operand *IRBuilder::neg(Operand *operand) { return unary<Neg>(operand); }

Operand *IRBuilder::not1(Operand *operand) { return unary<Not>(operand); }

Operand *IRBuilder::move(Operand *src) { return add(src, new Constant(0, src->type())); }

void IRBuilder::move(Operand *src, Operand *dst)
{
    _curr_block->append(new Add(dst, src, new Constant(0, src->type()), _curr_block));
}

void Function::set_cfg(Block *cfg) { _cfg->set_cfg(cfg); }