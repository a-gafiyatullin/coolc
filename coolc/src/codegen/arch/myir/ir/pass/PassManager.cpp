#include "PassManager.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"

using namespace myir;

void PassManager::PassManager::run()
{
    for (auto &[name, func] : _module.get<Function>())
    {
        for (auto *pass : _passes)
        {
            pass->run(func);
        }
    }
}

bool Pass::setup(Function *func)
{
    _func = func;
    _cfg = _func->cfg();

    if (_cfg->empty())
    {
        return false; // function was declared but not defined
    }

    Operand::set_id(_func->max_opnd_id());
    Instruction::set_id(_func->max_instruction_id());
    Block::set_id(_func->max_block_id());

    return true;
}

PassManager::~PassManager()
{
    for (auto *pass : _passes)
    {
        delete pass;
    }
}