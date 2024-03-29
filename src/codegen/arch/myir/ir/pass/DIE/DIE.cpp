#include "DIE.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"

using namespace myir;

void DIE::run(Function *func)
{
    std::vector<Instruction *> for_delete;

    for (auto *b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        for (auto *inst : b->insts())
        {
            if (!Instruction::isa<Store>(inst) && !Instruction::isa<Call>(inst))
            {
                if (inst->def() && inst->def()->uses().empty())
                {
                    for_delete.push_back(inst);
                }
            }
        }
    }

    Block::erase(for_delete);
}
