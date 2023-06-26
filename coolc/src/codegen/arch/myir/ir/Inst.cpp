#include "cfg/CFG.hpp"
#include "codegen/arch/myir/ir/Oper.hpp"
#include "utils/Utils.h"
#include <cassert>

using namespace myir;

int Instruction::ID = 0;

Instruction::Instruction(Operand *def, const std::vector<Operand *> &uses, Block *b)
    : _block(b), _def(def), _uses(ALLOC), _id(ID++)
{
    for (auto *use : uses)
    {
        if (use)
        {
            use->used_by(this);
            _uses.push_back(use);
        }
    }

    if (_def)
    {
        _def->defined_by(this);
    }
}

void Instruction::update_use(Operand *old_use, Operand *new_use)
{
    if (old_use == new_use)
    {
        return;
    }

    for (auto *&use : _uses)
    {
        if (use == old_use)
        {
            use = new_use;
            old_use->erase_use(this);
            new_use->used_by(this);
            return;
        }
    }

    SHOULD_NOT_REACH_HERE();
}

void Instruction::update_def(Operand *oper)
{
    if (_def)
    {
        _def->erase_def(this);
    }

    oper->defined_by(this);
    _def = oper;
}

void Phi::add_path(Operand *use, Block *b)
{
    _uses.push_back(use);
    use->used_by(this);
    _def_from_block[use] = b;
}

void Phi::update_use(Operand *old_use, Operand *new_use)
{
    Instruction::update_use(old_use, new_use);

    auto iter = _def_from_block.find(old_use);
    if (iter != _def_from_block.end())
    {
        auto *block = iter->second;
        _def_from_block.erase(iter);
        _def_from_block[new_use] = block;
    }
}