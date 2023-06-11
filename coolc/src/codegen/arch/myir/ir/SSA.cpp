#include "IR.hpp"
#include "codegen/arch/myir/ir/CFG.hpp"
#include "codegen/arch/myir/ir/CFG.inline.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/Inst.hpp"
#include "codegen/arch/myir/ir/Oper.hpp"
#include "utils/Utils.h"
#include <unordered_set>

using namespace myir;

void Function::construct_ssa()
{
    if (_cfg->empty())
    {
        return; // function was declared but not defined
    }

#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nSSA construction for " << short_name() << ":\n";
    }
#endif // DEBUG

    // 1.   insert phi-functions
    // 1.1. calcaulate DF
    auto &df = _cfg->dominance_frontier();
    // 1.2 find all variables with multiple defs
    auto defs = defs_in_blocks();
    // 1.3 insert
    insert_phis(defs, df);

    // 2. renaming phase
    std::unordered_map<Variable *, std::stack<Variable *>> varstacks;
    rename_phis(_cfg->root(), varstacks);

    // 3. pruning
    prune_ssa();

#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nAfter phi-functions insertion:\n" + dump() << std::endl;
    }
#endif // DEBUG
}

void Module::construct_ssa()
{
    for (auto &[name, func] : get<myir::Function>())
    {
        func->construct_ssa();
    }
}

std::unordered_map<Operand *, std::set<Block *>> Function::defs_in_blocks() const
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

void Function::insert_phis(const std::unordered_map<Operand *, std::set<Block *>> &vars_in_blocks,
                           const allocator::irunordered_map<Block *, allocator::irset<Block *>> &df)
{
    for (auto &[var, blocks] : vars_in_blocks)
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

void Function::rename_phis(Block *b, std::unordered_map<Variable *, std::stack<Variable *>> &varstacks)
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

            assert(isa<Variable>(def));
            auto *var = as<Variable>(def);
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
                if (isa<Variable>(use))
                {
                    // use without def - function formal. Skip this case.
                    // All other variables have to be defed before use
                    auto *var = as<Variable>(use);
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
                if (isa<Variable>(def))
                {
                    auto *var = as<Variable>(def);
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
            auto *def = as<Variable>((*i)->defs().at(0))->original_var();
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

void Function::prune_ssa()
{
    std::stack<Variable *> vars_stack;
    std::unordered_set<Variable *> alive_vars;

    prune_ssa_initialize(_cfg->root(), alive_vars, vars_stack);
    prune_ssa_propagate(alive_vars, vars_stack);
    prune_ssa_delete_dead_phis(alive_vars);
}

void Function::prune_ssa_initialize(Block *b, std::unordered_set<Variable *> &alive_vars,
                                    std::stack<Variable *> &vars_stack)
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
                        auto *var = Operand::as<Variable>(use);
                        alive_vars.insert(var); // mark as useful
                        vars_stack.push(var);
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
            prune_ssa_initialize(succ, alive_vars, vars_stack);
        }
    }
}

void Function::prune_ssa_propagate(std::unordered_set<Variable *> &alive_vars, std::stack<Variable *> &vars_stack)
{
    while (!vars_stack.empty())
    {
        auto *var = vars_stack.top();
        vars_stack.pop();

        auto *inst = var->defs().at(0);
        if (Instruction::isa<Phi>(inst))
        {
            for (auto *use : inst->uses())
            {
                auto *phiuse = Operand::as<Variable>(use);
                alive_vars.insert(phiuse);
                vars_stack.push(phiuse);
            }
        }
    }
}

void Function::prune_ssa_delete_dead_phis(std::unordered_set<Variable *> &alive_vars)
{
#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::string alive_vars_out = "Alive variables = [";

        for (auto *var : alive_vars)
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
                if (!alive_vars.contains(Operand::as<Variable>(inst->defs().at(0))))
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
void Function::dump_defs(const std::unordered_map<Operand *, std::set<Block *>> &defs)
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