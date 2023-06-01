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

#ifdef DEBUG
    if (TraceSSAConstruction)
    {
        std::cout << "\nAfter phi-functions insertion:\n" + dump() << std::endl;
    }
#endif // DEBUG
}

void Module::construct_ssa()
{
    for (auto &[name, func] : get<myir::func>())
    {
        func->construct_ssa();
    }
}

std::unordered_map<oper, std::set<block>> Function::defs_in_blocks() const
{
    std::unordered_map<oper, std::set<block>> var_to_blocks;

    for (auto b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        for (auto inst : b->insts())
        {
            for (auto def : inst->defs())
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

void Function::insert_phis(const std::unordered_map<oper, std::set<block>> &vars_in_blocks,
                           const std::unordered_map<block, std::set<block>> &df)
{
    for (auto &[var, blocks] : vars_in_blocks)
    {
        std::set<block> f;          // set of basic blocks where phi is added
        std::set<block> w = blocks; // set of basic blocks that contain definitions of v

        while (!w.empty())
        {
            auto x = *w.begin();
            w.erase(x);
            if (!df.contains(x))
            {
                continue; // no nodes in DF
            }

            for (auto y : df.at(x))
            {
                if (!f.contains(y))
                {
                    y->insts().insert(y->insts().begin(), IRBuilder::phi(var));
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

#ifdef DEBUG
void Function::dump_defs(const std::unordered_map<oper, std::set<block>> &defs)
{
    if (!TraceSSAConstruction)
    {
        return;
    }

    for (auto &[var, defs] : defs)
    {
        std::string s = "Variable " + var->name() + " was defed in [";
        for (auto b : defs)
        {
            s += b->name() + ", ";
        }

        trim(s, ", ");

        s += "]\n";

        std::cout << s;
    }
}
#endif // DEBUG