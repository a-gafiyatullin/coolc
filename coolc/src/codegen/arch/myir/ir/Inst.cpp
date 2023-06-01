#include "CFG.hpp"
#include "IR.hpp"
using namespace myir;

std::string Phi::dump() const
{
    assert(_defs.size() >= 1);
    auto result = _defs.at(0);

    std::string s = "phi " + result->name() + " <- [";
    for (auto &[def, block] : _def_from_block)
    {
        s += "(" + def->name() + ": " + block->name() + "), ";
    }

    trim(s, ", ");

    return s + "]";
}

std::string Store::dump() const
{
    assert(_uses.size() >= 3);

    auto base = _uses.at(0);
    auto offset = _uses.at(1);
    auto result = _uses.at(2);

    return "store " + result->name() + " -> [" + base->name() + " + " + offset->name() + "]";
}

std::string Load::dump() const
{
    assert(_uses.size() >= 2);
    assert(_defs.size() >= 1);

    auto base = _uses.at(0);
    auto offset = _uses.at(1);
    auto result = _defs.at(0);

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