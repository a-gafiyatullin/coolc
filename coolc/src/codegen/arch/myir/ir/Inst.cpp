#include "cfg/CFG.hpp"
#include <cassert>

using namespace myir;

Instruction::Instruction(const std::vector<Operand *> &defs, const std::vector<Operand *> &uses, Block *b)
    : _block(b), _defs(ALLOC), _uses(ALLOC)
{
    for (auto *use : uses)
    {
        if (use)
        {
            use->used_by(this);
            _uses.push_back(use);
        }
    }

    for (auto *def : defs)
    {
        if (def)
        {
            def->defed_by(this);
            _defs.push_back(def);
        }
    }
}

void Instruction::update_def(int i, Operand *oper)
{
    assert(i < _defs.size());
    assert(oper);

    oper->defed_by(this);
    _defs[i] = oper;
}

void Instruction::update_use(int i, Operand *oper)
{
    assert(i < _uses.size());
    assert(oper);

    oper->used_by(this);
    _uses[i] = oper;
}

std::string Phi::dump() const
{
    assert(_defs.size() >= 1);
    auto *result = _defs.at(0);

    std::string s = "phi " + result->name() + " <- [";
    for (auto &[def, block] : _def_from_block)
    {
        s += "(" + def->name() + ": " + block->name() + "), ";
    }

    trim(s, ", ");

    return s + "]";
}

void Phi::add_path(Operand *use, Block *b)
{
    _uses.push_back(use);
    _def_from_block[use] = b;
}

std::string Store::dump() const
{
    assert(_uses.size() >= 3);

    auto *base = _uses.at(0);
    auto *offset = _uses.at(1);
    auto *result = _uses.at(2);

    return "store " + result->name() + " -> [" + base->name() + " + " + offset->name() + "]";
}

std::string Load::dump() const
{
    assert(_uses.size() >= 2);
    assert(_defs.size() >= 1);

    auto *base = _uses.at(0);
    auto *offset = _uses.at(1);
    auto *result = _defs.at(0);

    return "load " + result->name() + " <- [" + base->name() + " + " + offset->name() + "]";
}

std::string Branch::dump() const { return "br " + _dest->name(); }

std::string CondBranch::dump() const
{
    assert(_uses.size() >= 1);
    return "br_cond " + _uses.at(0)->name() + "? " + _taken->name() + " : " + _not_taken->name();
}

std::string BinaryInst::print(const std::string &op) const
{
    assert(_uses.size() >= 2);
    assert(_defs.size() >= 1);
    return _defs.at(0)->name() + " <- " + _uses.at(0)->name() + " " + op + " " + _uses.at(1)->name();
}

std::string Sub::dump() const { return print("-"); }

std::string Add::dump() const { return print("+"); }

std::string Div::dump() const { return print("/"); }

std::string Mul::dump() const { return print("*"); }

std::string Xor::dump() const { return print("^"); }

std::string Or::dump() const { return print("|"); }

std::string Shl::dump() const { return print("<<"); }

std::string LT::dump() const { return print("<"); }

std::string LE::dump() const { return print("<="); }

std::string EQ::dump() const { return print("=="); }

std::string GT::dump() const { return print(">"); }

std::string UnaryInst::print(const std::string &op) const
{
    assert(_uses.size() >= 1);
    assert(_defs.size() >= 1);
    return _defs.at(0)->name() + " <- " + op + _uses.at(0)->name();
}

std::string Not::dump() const { return print("!"); }

std::string Neg::dump() const { return print("-"); }

std::string Call::dump() const
{
    std::string s = "call " + (_defs.size() > 0 ? _defs.at(0)->name() + " <- [" : "[");

    assert(_uses.size() >= 1);
    s += _uses.at(0)->name() + "](";

    for (auto u = _uses.begin() + 1; u != _uses.end(); u++)
    {
        s += (*u)->name() + ", ";
    }

    trim(s, ", ");

    return s + ")";
}

std::string Ret::dump() const { return "ret " + (!_uses.empty() ? _uses.at(0)->name() : ""); }