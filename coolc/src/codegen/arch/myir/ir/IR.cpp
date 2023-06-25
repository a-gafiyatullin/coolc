#include "IR.inline.hpp"
#include "cfg/CFG.inline.hpp"
#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"

using namespace myir;

int Operand::ID = 0;
int Block::ID = 0;

void IRBuilder::set_current_function(Function *func)
{
    if (_curr_func)
    {
        _curr_func->set_max_ids(Operand::max_id(), Instruction::max_id(), Block::max_id());
    }

    Operand::set_id();
    Instruction::set_id();
    Block::set_id();

    _curr_func = func;
}

void Function::set_max_ids(int max_opnd_id, int max_inst_id, int max_block_id)
{
    _max_block_id = max_block_id;
    _max_operand_id = max_opnd_id;
    _max_instruction_id = max_inst_id;
}

Function::Function(const std::string &name, const std::vector<Variable *> &params, OperandType return_type)
    : GlobalConstant(name, {}, POINTER), _params(params.begin(), params.end() ALLOCCOMMA), _return_type(return_type),
      _cfg(new CFG()), _max_operand_id(0), _max_instruction_id(0), _max_block_id(0)
{
}

void Operand::erase_def(Instruction *def)
{
    auto pos = std::find(_defs.begin(), _defs.end(), def);
    if (pos != _defs.end())
    {
        _defs.erase(pos);
    }
}

void Block::erase(Instruction *inst)
{
    // delete this instruction from uses and defs
    inst->def()->erase_def(inst);

    for (auto *use : inst->uses())
    {
        use->erase_use(inst);
    }

    _insts.erase(std::find(_insts.begin(), _insts.end(), inst));
}

void Block::erase(const std::vector<Instruction *> &insts)
{
    for (auto *inst : insts)
    {
        inst->holder()->erase(inst);
    }
}

void Block::append_before(Instruction *inst, Instruction *newinst)
{
    for (auto iter = _insts.begin(); iter != _insts.end(); iter++)
    {
        if (*iter == inst)
        {
            _insts.insert(iter, newinst);
            return;
        }
    }

    SHOULD_NOT_REACH_HERE();
}

void Block::append_instead(Instruction *inst, Instruction *newinst)
{
    if (_insts.empty())
    {
        return;
    }

    for (auto iter = _insts.begin(); iter != _insts.end(); iter++)
    {
        if (*iter == inst)
        {
            auto next = _insts.erase(iter);
            _insts.insert(next, newinst);
            return;
        }
    }

    SHOULD_NOT_REACH_HERE();
}

void Block::connect(Block *pred, Block *succ)
{
    assert(find(pred->_succs.begin(), pred->_succs.end(), succ) == pred->_succs.end());
    pred->_succs.push_back(succ);

    assert(find(succ->_preds.begin(), succ->_preds.end(), pred) == succ->_preds.end());
    succ->_preds.push_back(pred);
}

Operand *StructuredOperand::field(int offset) const
{
    switch (offset)
    {
    case codegen::HeaderLayoutOffsets::MarkOffset:
        return _fields.at(0);
    case codegen::HeaderLayoutOffsets::TagOffset:
        return _fields.at(1);
    case codegen::HeaderLayoutOffsets::SizeOffset:
        return _fields.at(2);
    case codegen::HeaderLayoutOffsets::DispatchTableOffset:
        return _fields.at(3);
    }

    int field_offset_from_header = offset - codegen::HeaderLayoutSizes::HeaderSize;
    assert(field_offset_from_header % WORD_SIZE == 0);
    assert(4 + field_offset_from_header / WORD_SIZE < _fields.size());

    return _fields.at(4 + field_offset_from_header / WORD_SIZE);
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

Operand *IRBuilder::move(Operand *src) { return unary<Move>(src); }

void IRBuilder::move(Operand *src, Operand *dst)
{
    assert(src);
    _curr_block->append(new Move(dst, src, _curr_block));
}

void Function::set_cfg(Block *cfg) { _cfg->set_cfg(cfg); }