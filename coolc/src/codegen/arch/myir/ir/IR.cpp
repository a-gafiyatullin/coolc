#include "IR.inline.hpp"
#include <stack>

using namespace myir;

uint64_t Operand::ID = 0;

std::string Block::dump() const
{
    std::string s = "  Block \"" + _name + "\", preds = [";

    for (auto p : _preds)
    {
        s += p->_name + ", ";
    }

    if (s.back() == ' ')
    {
        s = s.substr(0, s.length() - 2); // trim ", "
    }

    s += "], succs = [";

    for (auto p : _succs)
    {
        s += p->_name + ", ";
    }

    if (s.back() == ' ')
    {
        s = s.substr(0, s.length() - 2); // trim ", "
    }

    s += "]:\n";

    for (auto i : _insts)
    {
        s += "    " + i->dump() + "\n";
    }

    if (s.back() == '\n')
    {
        s = s.substr(0, s.length() - 1); // trim "\n"
    }

    return s;
}

void Block::connect(const block &pred, const block &succ)
{
    assert(find(pred->_succs.begin(), pred->_succs.end(), succ) == pred->_succs.end());
    pred->_succs.push_back(succ);

    assert(find(succ->_preds.begin(), succ->_preds.end(), pred) == succ->_preds.end());
    succ->_preds.push_back(pred);
}

std::vector<block> Block::preorder(const block &bl)
{
    std::stack<block> st;
    std::vector<block> traversal;

    st.push(bl);

    while (!st.empty())
    {
        auto b = st.top();
        st.pop();

        if (b->_is_visited)
        {
            continue;
        }

        b->_is_visited = true;

        traversal.push_back(b);

        for (auto succ : b->_succs)
        {
            st.push(succ);
        }
    }

    clear_visited(traversal);
    return traversal;
}

void Block::clear_visited(const std::vector<myir::block> &blocks)
{
    for (auto b : blocks)
    {
        b->_is_visited = false;
    }
}

void Block::postorder(const block &block, std::vector<myir::block> &blocks)
{
    if (block->_is_visited)
    {
        return;
    }

    block->_is_visited = true;

    for (auto s : block->_succs)
    {
        postorder(s, blocks);
    }

    blocks.push_back(block);
}

std::vector<block> Block::postorder(const block &block)
{
    std::vector<myir::block> traversal;
    postorder(block, traversal);

    clear_visited(traversal);
    return traversal;
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

    if (s.back() == ' ')
    {
        s = s.substr(0, s.length() - 2); // trim ", "
    }

    s += ")";

    return s;
}

std::string Function::short_name() const
{
    return _name;
}

std::string Function::dump() const
{
    if (!_cfg)
    {
        return name() + " {}";
    }

    std::string s = name() + " {\n";

    auto traverse = Block::postorder(_cfg);
    std::reverse(traverse.begin(), traverse.end());
    for (auto b : traverse)
    {
        s += b->dump() + "\n\n";
    }

    if (s.back() == '\n')
    {
        s = s.substr(0, s.length() - 2); // trim "\n\n"
    }

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

std::string Operand::dump() const
{
    return type_to_string(_type) + " " + _name;
}

std::string Constant::dump() const
{
    return std::to_string(_value);
}

std::string StructuredOperand::dump() const
{
    std::string s = "{";

    for (auto f : _fields)
    {
        s += f->name() + ", ";
    }

    if (s.back() == ' ')
    {
        s = s.substr(0, s.length() - 2); // trim ", "
    }

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

void IRBuilder::st(const oper &base, const oper &offset, const oper &value)
{
    auto inst = std::make_shared<Store>(base, offset, value);

    base->used_by(inst);
    offset->used_by(inst);
    value->used_by(inst);

    _curr_block->append(inst);
}

oper IRBuilder::call(const func &f, const std::vector<oper> &args)
{
    return call(f, f, args);
}

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

oper IRBuilder::add(const oper &lhs, const oper &rhs)
{
    return binary<Add>(lhs, rhs);
}

oper IRBuilder::sub(const oper &lhs, const oper &rhs)
{
    return binary<Sub>(lhs, rhs);
}

oper IRBuilder::div(const oper &lhs, const oper &rhs)
{
    return binary<Div>(lhs, rhs);
}

oper IRBuilder::mul(const oper &lhs, const oper &rhs)
{
    return binary<Mul>(lhs, rhs);
}

oper IRBuilder::shl(const oper &lhs, const oper &rhs)
{
    return binary<Shl>(lhs, rhs);
}

oper IRBuilder::lt(const oper &lhs, const oper &rhs)
{
    return binary<LT>(lhs, rhs);
}

oper IRBuilder::le(const oper &lhs, const oper &rhs)
{
    return binary<LE>(lhs, rhs);
}

oper IRBuilder::eq(const oper &lhs, const oper &rhs)
{
    return binary<EQ>(lhs, rhs);
}

oper IRBuilder::gt(const oper &lhs, const oper &rhs)
{
    return binary<GT>(lhs, rhs);
}

oper IRBuilder::or2(const oper &lhs, const oper &rhs)
{
    return binary<Or>(lhs, rhs);
}

oper IRBuilder::xor2(const oper &lhs, const oper &rhs)
{
    return binary<Xor>(lhs, rhs);
}

oper IRBuilder::neg(const oper &operand)
{
    return unary<Neg>(operand);
}

oper IRBuilder::not1(const oper &operand)
{
    return unary<Not>(operand);
}

oper IRBuilder::move(const oper &src)
{
    return add(src, Constant::constant(src->type(), 0));
}

void IRBuilder::move(const oper &src, const oper &dst)
{
    auto inst = std::make_shared<Add>(dst, src, Constant::constant(src->type(), 0));

    src->used_by(inst);
    dst->defed_by(inst);

    _curr_block->append(inst);
}