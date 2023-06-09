#include "IR.hpp"
#include "codegen/arch/myir/ir/CFG.inline.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "utils/Utils.h"

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
                           const std::unordered_map<Block *, std::set<Block *>> &df)
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
    if (!b)
    {
        return;
    }

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

            i->defs()[0] = newvar;
            varstacks[var].push(newvar);
            popcount[var]++;
        }
        else
        {
            // Rewrite uses of global names with the current version
            for (auto *&use : i->uses())
            {
                if (isa<Variable>(use))
                {
                    // use without def - formal
                    if (!varstacks[as<Variable>(use)].empty())
                    {
                        use = varstacks[as<Variable>(use)].top();
                    }
                }
            }

            // Rewrite definition by inventing & pushing new name
            for (auto *&def : i->defs())
            {
                if (isa<Variable>(def))
                {
                    auto *var = as<Variable>(def);
                    auto *newvar = new Variable(*var);

                    def = newvar;
                    varstacks[var].push(newvar);
                    popcount[var]++;
                }
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
    for (auto *dsucc : _cfg->dominator_tree()[b])
    {
        rename_phis(dsucc, varstacks);
    }

    // < on exit from b > pop names generated in b from stacks
    for (auto [var, count] : popcount)
    {
        for (int i = 0; i < count; i++)
        {
            varstacks[var].pop();
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