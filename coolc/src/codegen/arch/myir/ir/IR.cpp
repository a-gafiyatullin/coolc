#include "IR.inline.hpp"
#include "codegen/arch/myir/ir/CFG.hpp"
using namespace myir;

int Operand::ID = 0;

Function::Function(const std::string &name, const std::vector<oper> &params, OperandType return_type)
    : GlobalConstant(name, {}, POINTER), _params(params), _return_type(return_type), _is_leaf(false),
      _cfg(std::make_shared<CFG>())
{
}

std::string Block::dump() const
{
    std::string s = "  Block \"" + _name + "\", preds = [";

    for (auto p : _preds)
    {
        s += p->_name + ", ";
    }

    trim(s, ", ");

    s += "], succs = [";

    for (auto p : _succs)
    {
        s += p->_name + ", ";
    }

    trim(s, ", ");

    s += "]:\n";

    for (auto i : _insts)
    {
        s += "    " + i->dump() + "\n";
    }

    trim(s, "\n");

    return s;
}

void Block::connect(const block &pred, const block &succ)
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
    std::string s = type_to_string(_return_type) + " " + _name + "(";

    for (auto p : _params)
    {
        s += p->dump() + ", ";
    }

    trim(s, ", ");

    return s + ")";
}

std::string Function::short_name() const { return _name; }

std::string Function::dump() const
{
    if (_cfg->empty())
    {
        return name() + " {}";
    }

    std::string s = name() + " {\n";

    for (auto b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
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

std::string StructuredOperand::dump() const
{
    std::string s = "{";

    for (auto f : _fields)
    {
        s += f->name() + ", ";
    }

    trim(s, ", ");

    s += "} " + _name;

    return s;
}

void IRBuilder::ret(const oper &value)
{
    inst instr = nullptr;
    if (value)
    {
        instr = std::make_shared<Ret>(value);
        value->used_by(instr);
    }
    else
    {
        instr = std::make_shared<Ret>();
    }

    _curr_block->append(instr);
}

inst IRBuilder::phi(const oper &var)
{
    auto inst = std::make_shared<Phi>(var);
    var->defed_by(inst);

    return inst;
}

void IRBuilder::st(const oper &base, const oper &offset, const oper &value)
{
    auto inst = std::make_shared<Store>(base, offset, value);

    base->used_by(inst);
    offset->used_by(inst);
    value->used_by(inst);

    _curr_block->append(inst);
}

oper IRBuilder::call(const func &f, const std::vector<oper> &args) { return call(f, f, args); }

oper IRBuilder::call(const func &f, const oper &dst, const std::vector<oper> &args)
{
    std::vector<oper> uses;
    uses.push_back(dst);
    uses.insert(uses.end(), args.begin(), args.end());

    inst inst = nullptr;
    oper retval = nullptr;

    if (f->has_return())
    {
        retval = Operand::operand(f->return_type());
        inst = std::make_shared<Call>(f, retval, uses);

        retval->defed_by(inst);
    }
    else
    {
        inst = std::make_shared<Call>(f, uses);
    }

    for (auto a : args)
    {
        a->used_by(inst);
    }

    _curr_block->append(inst);

    return retval;
}

void IRBuilder::cond_br(const oper &pred, const block &taken, const block &fall_through)
{
    auto inst = std::make_shared<CondBranch>(pred, taken, fall_through);
    pred->used_by(inst);

    _curr_block->append(inst);

    Block::connect(_curr_block, taken);
    Block::connect(_curr_block, fall_through);
}

void IRBuilder::br(const block &taken)
{
    auto inst = std::make_shared<Branch>(taken);
    _curr_block->append(inst);

    Block::connect(_curr_block, taken);
}

oper IRBuilder::add(const oper &lhs, const oper &rhs) { return binary<Add>(lhs, rhs); }

oper IRBuilder::sub(const oper &lhs, const oper &rhs) { return binary<Sub>(lhs, rhs); }

oper IRBuilder::div(const oper &lhs, const oper &rhs) { return binary<Div>(lhs, rhs); }

oper IRBuilder::mul(const oper &lhs, const oper &rhs) { return binary<Mul>(lhs, rhs); }

oper IRBuilder::shl(const oper &lhs, const oper &rhs) { return binary<Shl>(lhs, rhs); }

oper IRBuilder::lt(const oper &lhs, const oper &rhs) { return binary<LT>(lhs, rhs); }

oper IRBuilder::le(const oper &lhs, const oper &rhs) { return binary<LE>(lhs, rhs); }

oper IRBuilder::eq(const oper &lhs, const oper &rhs) { return binary<EQ>(lhs, rhs); }

oper IRBuilder::gt(const oper &lhs, const oper &rhs) { return binary<GT>(lhs, rhs); }

oper IRBuilder::or2(const oper &lhs, const oper &rhs) { return binary<Or>(lhs, rhs); }

oper IRBuilder::xor2(const oper &lhs, const oper &rhs) { return binary<Xor>(lhs, rhs); }

oper IRBuilder::neg(const oper &operand) { return unary<Neg>(operand); }

oper IRBuilder::not1(const oper &operand) { return unary<Not>(operand); }

oper IRBuilder::move(const oper &src) { return add(src, Constant::constval(src->type(), 0)); }

void IRBuilder::move(const oper &src, const oper &dst)
{
    auto inst = std::make_shared<Add>(dst, src, Constant::constval(src->type(), 0));

    src->used_by(inst);
    dst->defed_by(inst);

    _curr_block->append(inst);
}

void Function::set_cfg(const block &cfg) { _cfg->set_cfg(cfg); }