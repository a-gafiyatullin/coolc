#include "CP.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
using namespace myir;

void CP::run(Function *func)
{
    std::unordered_map<Operand *, Operand *> copies;
    sparse_data_flow_propagation([this, &copies](Instruction *inst, std::stack<Instruction *> &ssa_worklist,
                                                 std::stack<Block *> &cfg_worklist, std::vector<bool> &bvisited) {
        if (Instruction::isa<Phi>(inst))
        {
            auto executable = operands_from_executable_paths(inst, bvisited);

            // get optimisitic prediction
            Operand *copy = nullptr;
            for (auto *o : executable)
            {
                if (!copy)
                {
                    copy = copies[o];
                }
                else if (copy != copies[o])
                {
                    // values doesn't match previously found value for this def
                    copy = nullptr;
                    break;
                }
            }

            if (!copy)
            {
                copy = inst->def();
            }

            copies[inst->def()] = copy;
        }
        else if (Instruction::isa<CondBranch>(inst))
        {
            // TODO: optimize here
            auto *br = Instruction::as<CondBranch>(inst);

            cfg_worklist.push(br->taken());
            cfg_worklist.push(br->not_taken());
        }
        else if (Instruction::isa<Move>(inst))
        {
            auto *use = inst->use(0);
            copies[inst->def()] = use;

            // nobody defines constants
            if (Operand::isa<Constant>(use) || Operand::isa<GlobalConstant>(use))
            {
                copies[use] = use;
            }
        }
        else
        {
            if (inst->def())
            {
                copies[inst->def()] = inst->def();
            }
        }
    });

    eliminate_copies(copies);
}

void CP::eliminate_copies(std::unordered_map<Operand *, Operand *> &copies)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (auto &[dst, src] : copies)
        {
            Operand *best = src;
            while (copies[best] != best)
            {
                best = copies[best];
                changed = true;
            }

            assert(best);
            src = best;
        }
    }

    for (auto &[dst, src] : copies)
    {
        if (dst != src)
        {
            // TODO: bad coding style
            std::vector<Instruction *> for_update;
            for (auto *use : dst->uses())
            {
                for_update.push_back(use);
            }

            for (auto *inst : for_update)
            {
                inst->update_use(dst, src);
            }
        }
    }
}