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
            if (!func->cfg()->empty())
            {
                pass->_func = func;
                pass->_cfg = func->cfg();

                func->reset_max_ids();
                pass->run(func);
                func->record_max_ids();
            }
        }
    }
}

std::vector<Operand *> Pass::operands_from_executable_paths(Instruction *inst, std::vector<bool> &bvisited)
{
    // use data flow info only from executed paths
    auto *phi = Instruction::as<Phi>(inst);
    std::vector<Operand *> executable;

    for (auto *b : inst->holder()->preds())
    {
        if (bvisited[b->id()])
        {
            executable.push_back(phi->oper_path(b));
        }
    }

    return executable;
}

void Pass::append_uses_to_worklist(Operand *operand, std::stack<Instruction *> &ssa_worklist)
{
    std::for_each(operand->uses().begin(), operand->uses().end(),
                  [&ssa_worklist](Instruction *inst) { ssa_worklist.push(inst); });
}

void Pass::sparse_data_flow_propagation(
    const std::function<void(Instruction *, std::stack<Instruction *> &ssa_worklist, std::stack<Block *> &cfg_worklist,
                             std::vector<bool> &bvisited)> &visitor)
{
    assert(Block::max_id());
    std::vector<bool> bvisited(Block::max_id());

    std::stack<Block *> cfg_worklist;
    cfg_worklist.push(_cfg->root());

    std::stack<Instruction *> ssa_worklist;

    while (!ssa_worklist.empty() || !cfg_worklist.empty())
    {
        if (!cfg_worklist.empty())
        {
            auto *b = cfg_worklist.top();
            cfg_worklist.pop();

            for (auto *inst : b->insts())
            {
                if (Instruction::isa<Phi>(inst))
                {
                    visitor(inst, ssa_worklist, cfg_worklist, bvisited);
                }
                else
                {
                    break;
                }
            }

            if (!bvisited[b->id()])
            {
                bvisited[b->id()] = true;

                std::for_each(b->insts().begin(), b->insts().end(),
                              [&visitor, &ssa_worklist, &cfg_worklist, &bvisited](Instruction *i) {
                                  if (!Instruction::isa<Phi>(i))
                                  {
                                      visitor(i, ssa_worklist, cfg_worklist, bvisited);
                                  }
                              });
            }

            if (b->succs().size() == 1 && !bvisited[b->succ(0)->id()])
            {
                cfg_worklist.push(b->succ(0));
            }
        }
        else if (!ssa_worklist.empty())
        {
            auto *inst = ssa_worklist.top();
            ssa_worklist.pop();

            if (Instruction::isa<Phi>(inst))
            {
                visitor(inst, ssa_worklist, cfg_worklist, bvisited);
            }
            else
            {
                if (std::any_of(inst->holder()->preds().begin(), inst->holder()->preds().end(),
                                [&bvisited](Block *b) { return bvisited[b->id()]; }))
                {
                    visitor(inst, ssa_worklist, cfg_worklist, bvisited);
                }
            }
        }
    }
}

void Pass::merge_blocks()
{
    std::vector<Block *> for_delete;

    bool cont = true;

    // TODO: too slow?
    while (cont)
    {
        cont = false;
        for (auto *bb : _cfg->traversal<CFG::REVERSE_POSTORDER>())
        {
            if (bb->insts().size() == 0 || !Instruction::isa<Branch>(bb->insts().back()))
            {
                continue;
            }

            auto *successor = bb->succ(0);

            if (successor->preds().size() != 1)
            {
                continue;
            }

            assert(bb == successor->preds().at(0));

            // empty block
            if (successor->insts().size() == 0)
            {
                continue;
            }

            cont = true;

            // delete branch
            bb->erase(bb->insts().back());

            // unconditional branch and successor has only one predecessor
            for (auto *inst : successor->insts())
            {
                bb->append(inst);
            }

            successor->clear();
            for_delete.push_back(successor);
        }

        for (auto *b : for_delete)
        {
            b->disconnect();
        }
    }
}

PassManager::~PassManager()
{
    for (auto *pass : _passes)
    {
        delete pass;
    }
}
