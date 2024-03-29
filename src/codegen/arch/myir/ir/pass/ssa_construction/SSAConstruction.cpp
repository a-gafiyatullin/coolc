#include "SSAConstruction.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/Oper.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.inline.hpp"
#include "utils/Utils.h"
#include <iostream>

using namespace myir;

void SSAConstruction::run(Function *func)
{
#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nSSA construction for " << _func->short_name() << ":\n";
    }
#endif // DEBUG

    auto dominfo = _cfg->dominance();

    // 1. insert phis phase
    insert_phis(dominfo);

    // 2. renaming phase
    std::unordered_map<Variable *, std::stack<Operand *>> varstacks;
    // formals of the method - first defs
    for (int i = 0; i < _func->params_size(); i++)
    {
        // replace formals with tmp, because they have correct id
        auto *newvar = new Variable(*_func->param(i));
        varstacks[_func->param(i)].push(newvar);
        _func->set_param(i, newvar);
    }
    rename_phis(dominfo, _cfg->root(), varstacks);

    // 3. pruning phase
    prune_ssa(dominfo);

#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nAfter phi-functions insertion:\n" + _func->dump() << std::endl;
    }
#endif // DEBUG
}

std::unordered_map<Operand *, std::set<Block *>> SSAConstruction::defs_in_blocks() const
{
    std::unordered_map<Operand *, std::set<Block *>> var_to_blocks;

    for (auto *b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        for (auto *inst : b->insts())
        {
            if (Operand::isa<Variable>(inst->def()))
            {
                var_to_blocks[inst->def()].insert(b);
            }
        }
    }

    DEBUG_ONLY(dump_defs(var_to_blocks));

    return var_to_blocks;
}

void SSAConstruction::insert_phis(const CFG::DominanceInfo &info)
{
    // 1.1. calculate DF
    auto &df = info.dominance_frontier();
    // 1.2 find all variables with multiple defs
    auto defs = defs_in_blocks();

    for (auto &[var, blocks] : defs)
    {
        std::set<Block *> f;          // set of basic blocks where phi is added
        std::set<Block *> w = blocks; // set of basic blocks that contain definitions of v

        while (!w.empty())
        {
            auto *x = *w.begin();
            w.erase(x);
            if (!df.contains(x))
            {
                continue; // no nodes in DF
            }

            for (auto *y : df.at(x))
            {
                if (!f.contains(y))
                {
                    IRBuilder::phi(var, y);
                    f.insert(y);
                    if (!blocks.contains(y))
                    {
                        w.insert(y);
                    }
                }
            }
        }
    }
}

void SSAConstruction::rename_phis(const CFG::DominanceInfo &info, Block *b,
                                  std::unordered_map<Variable *, std::stack<Operand *>> &varstacks)
{
    assert(b);

    // track how variable stack has changed during this recursion call
    std::unordered_map<Variable *, int> popcount;

    for (auto *i : b->insts())
    {
        // generate unique name for each phi-function
        if (Instruction::isa<Phi>(i))
        {
            auto *var = Operand::as<Variable>(i->def());
            auto *newvar = new Variable(*var);

            i->update_def(newvar);
            varstacks[var].push(newvar);
            popcount[var]++;
        }
        else
        {
            // Rewrite uses of global names with the current version
            for (auto *use : i->uses())
            {
                if (Operand::isa<Variable>(use))
                {
                    // use without def - function formal. Skip this case.
                    // All other variables have to be defined before use
                    auto *var = Operand::as<Variable>(use);
                    if (!varstacks[var].empty())
                    {
                        i->update_use(use, varstacks[var].top());
                    }
                }
            }

            // Rewrite definition by inventing & pushing new name
            if (Operand::isa<Variable>(i->def()))
            {
                auto *var = Operand::as<Variable>(i->def());
                auto *newvar = new Variable(*var);

                i->update_def(newvar);
                varstacks[var].push(newvar);
                popcount[var]++;
            }
        }
    }

    // fill in phi-function parameters of successor blocks
    for (auto *succ : b->succs())
    {
        auto i = succ->insts().begin();
        while (Instruction::isa<Phi>(*i))
        {
            auto *def = Operand::as<Variable>((*i)->def())->original_var();
            if (varstacks.contains(def))
            {
                assert(!varstacks[def].empty());
                Instruction::as<Phi>(*i)->add_path(varstacks[def].top(), b);
            }

            i++;
        }
    }

    if (info.dominator_tree().contains(b))
    {
        // recurse on b’s children in the dominance tree
        for (auto *dsucc : info.dominator_tree().at(b))
        {
            rename_phis(info, dsucc, varstacks);
        }
    }

    // on exit from b pop names generated in b from stacks
    for (auto [var, count] : popcount)
    {
        for (int i = 0; i < count; i++)
        {
            varstacks[var].pop();
        }

        if (varstacks[var].empty())
        {
            varstacks.erase(var);
        }
    }
}

void SSAConstruction::prune_ssa(const CFG::DominanceInfo &info)
{
    std::stack<Operand *> varsstack;
    std::vector<bool> alivevars(Operand::max_id());

    prune_ssa_initialize(info, _cfg->root(), alivevars, varsstack);
    prune_ssa_propagate(alivevars, varsstack);
    prune_ssa_delete_dead_phis(alivevars);
}

void SSAConstruction::prune_ssa_initialize(const CFG::DominanceInfo &info, Block *b, std::vector<bool> &alivevars,
                                           std::stack<Operand *> &varstack)
{
    for (auto *i : b->insts())
    {
        if (!Instruction::isa<Phi>(i))
        {
            for (auto *use : i->uses())
            {
                // only variables bother us
                if (Operand::isa<Variable>(use))
                {
                    // use defined by phi function
                    if (use->has_def() && Instruction::isa<Phi>(use->def()))
                    {
                        alivevars[use->id()] = true; // mark as useful
                        varstack.push(use);
                    }
                }
            }
        }
    }

    // dominance order
    auto &dt = info.dominator_tree();
    if (dt.contains(b))
    {
        for (auto *succ : dt.at(b))
        {
            prune_ssa_initialize(info, succ, alivevars, varstack);
        }
    }
}

void SSAConstruction::prune_ssa_propagate(std::vector<bool> &alivevars, std::stack<Operand *> &varstack)
{
    while (!varstack.empty())
    {
        auto *var = varstack.top();
        varstack.pop();

        // no defs - function formal
        if (!var->has_def())
        {
            continue;
        }

        auto *inst = var->def();
        if (Instruction::isa<Phi>(inst))
        {
            for (auto *use : inst->uses())
            {
                if (!alivevars[use->id()])
                {
                    alivevars[use->id()] = true;
                    varstack.push(use);
                }
            }
        }
    }
}

void SSAConstruction::prune_ssa_delete_dead_phis(const std::vector<bool> &alivevars)
{
    std::vector<Instruction *> for_delete;
    for (auto *bb : _cfg->traversal<CFG::PREORDER>())
    {
        for (auto *inst : bb->insts())
        {
            // destination operand of I marked as useless
            if (!Instruction::isa<Phi>(inst))
            {
                break;
            }

            if (!alivevars[inst->def()->id()])
            {
                for_delete.push_back(inst);
            }
        }
    }

    Block::erase(for_delete);
}

#ifdef DEBUG
void SSAConstruction::dump_defs(const std::unordered_map<Operand *, std::set<Block *>> &defs)
{
    if (!TraceSSAConstruction)
    {
        return;
    }

    for (auto &[var, defs] : defs)
    {
        std::string s = "Variable " + var->name() + " was defined in [";
        for (auto *b : defs)
        {
            s += b->name() + ", ";
        }

        trim(s, ", ");

        s += "]\n";

        std::cout << s;
    }
}
#endif // DEBUG
