#include "SSAConstruction.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/Oper.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.inline.hpp"
#include "utils/Utils.h"

using namespace myir;

void SSAConstruction::run(Function *func)
{
    _func = func;
    _cfg = _func->cfg();

    if (_cfg->empty())
    {
        return; // function was declared but not defined
    }

#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nSSA construction for " << _func->short_name() << ":\n";
    }
#endif // DEBUG

    // 1. insert phis phase
    insert_phis();

    // 2. renaming phase
    std::unordered_map<Variable *, std::stack<Operand *>> varstacks;
    rename_phis(_cfg->root(), varstacks);

    // 3. pruning phase
    prune_ssa();

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
            for (auto *def : inst->defs())
            {
                if (Operand::isa<Variable>(def))
                {
                    var_to_blocks[def].insert(b);
                }
            }
        }
    }

    DEBUG_ONLY(dump_defs(var_to_blocks));

    return var_to_blocks;
}

void SSAConstruction::insert_phis()
{
    // 1.1. calcaulate DF
    auto &df = _cfg->dominance_frontier();
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

void SSAConstruction::rename_phis(Block *b, std::unordered_map<Variable *, std::stack<Operand *>> &varstacks)
{
    assert(b);

    // track how variable stack has changed during this recursion call
    std::unordered_map<Variable *, int> popcount;

    for (auto *i : b->insts())
    {
        // generate unique name for each phi-function
        if (Instruction::isa<Phi>(i))
        {
            auto *def = i->defs().at(0);

            auto *var = Operand::as<Variable>(def);
            auto *newvar = new Variable(*var);

            i->update_def(0, newvar);
            varstacks[var].push(newvar);
            popcount[var]++;
        }
        else
        {
            // Rewrite uses of global names with the current version
            int num = 0;
            for (auto *use : i->uses())
            {
                if (Operand::isa<Variable>(use))
                {
                    // use without def - function formal. Skip this case.
                    // All other variables have to be defed before use
                    auto *var = Operand::as<Variable>(use);
                    if (!varstacks[var].empty())
                    {
                        i->update_use(num, varstacks[var].top());
                    }
                }
                num++;
            }

            // Rewrite definition by inventing & pushing new name
            num = 0;
            for (auto *def : i->defs())
            {
                if (Operand::isa<Variable>(def))
                {
                    auto *var = Operand::as<Variable>(def);
                    auto *newvar = new Variable(*var);

                    i->update_def(num, newvar);
                    varstacks[var].push(newvar);
                    popcount[var]++;
                }
                num++;
            }
        }
    }

    // fill in phi-function parameters of successor blocks
    for (auto *succ : b->succs())
    {
        auto i = succ->insts().begin();
        while (Instruction::isa<Phi>(*i))
        {
            auto *def = Operand::as<Variable>((*i)->defs().at(0))->original_var();
            if (varstacks.contains(def))
            {
                assert(!varstacks[def].empty());
                Instruction::as<Phi>(*i)->add_path(varstacks[def].top(), b);
            }

            i++;
        }
    }

    // recurse on b’s children in the dominance tree
    if (_cfg->dominator_tree().contains(b))
    {
        for (auto *dsucc : _cfg->dominator_tree().at(b))
        {
            rename_phis(dsucc, varstacks);
        }
    }

    // on exit from b pop names generated in b from stacks
    for (auto [var, count] : popcount)
    {
        for (int i = 0; i < count; i++)
        {
            varstacks[var].pop();
        }
    }
}

void SSAConstruction::prune_ssa()
{
    std::stack<Operand *> varsstack;
    std::unordered_set<Operand *> alivevars;

    prune_ssa_initialize(_cfg->root(), alivevars, varsstack);
    prune_ssa_propagate(alivevars, varsstack);
    prune_ssa_delete_dead_phis(alivevars);
}

void SSAConstruction::prune_ssa_initialize(Block *b, std::unordered_set<Operand *> &alivevars,
                                           std::stack<Operand *> &varsstack)
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
                    // dominance property and function formal case
                    assert(use->defs().size() <= 1);

                    // use defined by phi function
                    if (!use->defs().empty() && Instruction::isa<Phi>(use->defs().at(0)))
                    {
                        alivevars.insert(use); // mark as useful
                        varsstack.push(use);
                    }
                }
            }
        }
    }

    // dominance order
    auto &dt = _cfg->dominator_tree();
    if (dt.contains(b))
    {
        for (auto *succ : dt.at(b))
        {
            prune_ssa_initialize(succ, alivevars, varsstack);
        }
    }
}

void SSAConstruction::prune_ssa_propagate(std::unordered_set<Operand *> &alivevars, std::stack<Operand *> &varsstack)
{
    while (!varsstack.empty())
    {
        auto *var = varsstack.top();
        varsstack.pop();

        auto *inst = var->defs().at(0);
        if (Instruction::isa<Phi>(inst))
        {
            for (auto *use : inst->uses())
            {
                alivevars.insert(use);
                varsstack.push(use);
            }
        }
    }
}

void SSAConstruction::prune_ssa_delete_dead_phis(std::unordered_set<Operand *> &alivevars)
{
#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::string alive_vars_out = "Alive variables = [";

        for (auto *var : alivevars)
        {
            alive_vars_out += var->name() + ", ";
        }

        trim(alive_vars_out, ", ");
        alive_vars_out += "]";

        std::cout << alive_vars_out << std::endl;
    }
#endif // DEBUG

    for (auto *bb : _cfg->traversal<CFG::PREORDER>())
    {
        for (auto inst_iter = bb->insts().begin(); inst_iter != bb->insts().end();)
        {
            auto *inst = *inst_iter;
            // destination operand of I marked as useless
            if (Instruction::isa<Phi>(inst))
            {
                if (!alivevars.contains(inst->defs().at(0)))
                {
                    inst_iter = bb->insts().erase(inst_iter);
                }
                else
                {
                    inst_iter++;
                }
            }
            else
            {
                break;
            }
        }
    }
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
        std::string s = "Variable " + var->name() + " was defed in [";
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