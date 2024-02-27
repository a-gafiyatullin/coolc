#include "IR.inline.hpp"
#include "cfg/CFG.hpp"
#include <cassert>
#include <iostream>

using namespace myir;

std::string StructuredOperand::dump() const
{
    std::string s = "{";

    for (auto *f : _fields)
    {
        s += f->name() + ", ";
    }

    trim(s, ", ");

    s += "} " + name();

    return s;
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

std::string type_to_string(OperandType type)
{
    const static std::string OPERAND_TYPE_NAME[] = {"int8",  "uint8",   "int32",   "uint32", "int64",     "uint64",
                                                    "void*", "integer", "boolean", "string", "structure", "void"};
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

std::string Module::dump()
{
    std::string s;

    for (auto &[name, g] : _constants)
    {
        s += g->dump() + "\n\n";
    }

    for (auto &[name, g] : _variables)
    {
        s += g->dump() + "\n\n";
    }

    for (auto &[name, f] : _funcs)
    {
        f->reset_max_ids();
        s += f->dump() + "\n\n";
    }

    return s;
}

std::string Operand::dump() const { return type_to_string(_type) + " " + name(); }

std::string Constant::name() const { return std::to_string(_value); }

std::string Variable::name() const
{
    return Operand::name() + (this != _original_var ? "[" + _original_var->name() + "]" : "");
}

std::string Phi::dump() const
{
    std::string s = "phi " + _def->name() + " <- [";
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

    auto *base = _uses.at(0);
    auto *offset = _uses.at(1);
    auto *result = _uses.at(2);

    return "store " + result->name() + " -> [" + base->name() + " + " + offset->name() + "]";
}

std::string Load::dump() const
{
    assert(_uses.size() >= 2);
    auto *base = _uses.at(0);
    auto *offset = _uses.at(1);

    return "load " + _def->name() + " <- [" + base->name() + " + " + offset->name() + "]";
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
    return _def->name() + " <- " + _uses.at(0)->name() + " " + op + " " + _uses.at(1)->name();
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
    return _def->name() + " <- " + op + _uses.at(0)->name();
}

std::string Not::dump() const { return print("!"); }

std::string Neg::dump() const { return print("-"); }

std::string Move::dump() const { return print(""); }

std::string Call::dump() const
{
    std::string s = "call " + (_def != nullptr ? _def->name() + " <- [" : "[");

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
