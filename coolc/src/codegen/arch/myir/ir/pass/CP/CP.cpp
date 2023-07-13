#include "CP.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
using namespace myir;

void CP::run(Function *func)
{
    // map copies to sources
    std::unordered_map<Operand *, Operand *> copies;

    // parameters are source values
    for (auto *param : _func->params())
    {
        copies[param] = param;
    }

    // find all copies
    sparse_data_flow_propagation([this, &copies](Instruction *inst, std::stack<Instruction *> &ssa_worklist,
                                                 std::stack<Block *> &cfg_worklist, std::vector<bool> &bvisited) {
        if (Instruction::isa<Phi>(inst))
        {
            // operands from executable paths
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
                    // new value doesn't match previously found value for this def
                    copy = nullptr;
                    break;
                }
            }

            auto *def = inst->def();

            if (!copy)
            {
                copy = def;
            }

            // for this copy source either the copy itself or phi function can be eliminated
            if (copies[def] != copy)
            {
                copies[def] = copy;
                append_uses_to_worklist(def, ssa_worklist);
            }
        }
        else if (Instruction::isa<CondBranch>(inst))
        {
            // TODO: opportunity for optimization here, but requires computation of conditions in compile time
            auto *br = Instruction::as<CondBranch>(inst);

            // append the CFG edges that were non-executable to the cfg_worklist
            if (!bvisited[br->taken()->id()])
            {
                cfg_worklist.push(br->taken());
            }

            if (!bvisited[br->not_taken()->id()])
            {
                cfg_worklist.push(br->not_taken());
            }
        }
        else if (Instruction::isa<Move>(inst))
        {
            // Move instruction is just a copy
            auto *use = inst->use(0);
            auto *def = inst->def();

            if (copies[def] != use)
            {
                copies[def] = use;
                append_uses_to_worklist(def, ssa_worklist);
            }

            // Nobody defines constants and parameters, but we need them in the map (see CP::eliminate_copies)
            if (copies[use] != use && !use->has_def())
            {
                copies[use] = use;
                // Constants are not SSA values, so no need append_uses_to_worklist
            }
        }
        else
        {
            // Various instructions that create a new temporary (calls, loads and etc)
            auto *def = inst->def();

            if (def)
            {
                if (copies[def] != def)
                {
                    copies[def] = def;
                    append_uses_to_worklist(def, ssa_worklist);
                }
            }
        }
    });

    eliminate_copies(copies);
}

void CP::eliminate_copies(std::unordered_map<Operand *, Operand *> &copies)
{
    // Find source for every copy
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

    // Replace copies with their original values
    for (auto &[dst, src] : copies)
    {
        if (dst != src)
        {
            // Instruction::update_use modifies iterators. Need to copy here
            std::vector<Instruction *> for_update(dst->uses().begin(), dst->uses().end());

            for (auto *inst : for_update)
            {
                inst->update_use(dst, src);
            }
        }
    }
}