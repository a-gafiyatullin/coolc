#include "NCE.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
using namespace myir;

void NCE::run(Function *func)
{
    assert(Operand::max_id());
    std::vector<bool> not_null(Operand::max_id());

    gather_not_nulls(not_null);
    sparse_data_flow_propagation([this, &not_null](Instruction *inst, std::stack<Instruction *> &ssa_worklist,
                                                   std::stack<Block *> &cfg_worklist, std::vector<bool> &bvisited) {
        auto is_not_null = [&not_null](Operand *oper) {
            return Operand::isa<GlobalConstant>(oper) || not_null[oper->id()];
        };

        if (Instruction::isa<Phi>(inst))
        {
            auto executable = operands_from_executable_paths(inst, bvisited);

            // get optimisitic prediction
            bool not_null_state = true;
            for (auto *o : executable)
            {
                not_null_state &= is_not_null(o);
            }

            auto *def = inst->def();
            if (not_null[def->id()] != not_null_state)
            {
                not_null[def->id()] = not_null_state;
                append_uses_to_worklist(def, ssa_worklist);
            }
        }
        else if (Instruction::isa<CondBranch>(inst))
        {
            // use the best estimaton for this branch
            auto *br = Instruction::as<CondBranch>(inst);
            if (!br->use(0)->has_def())
            {
                // know path exactly
                if (Operand::isa<Constant>(br->use(0)))
                {
                    if (Operand::as<Constant>(br->use(0))->value() != 0)
                    {
                        // only taken is executable
                        if (!bvisited[br->taken()->id()])
                        {
                            cfg_worklist.push(br->taken());
                        }
                    }
                    else
                    {
                        // only not taken is executable
                        if (!bvisited[br->not_taken()->id()])
                        {
                            cfg_worklist.push(br->not_taken());
                        }
                    }
                }
                else
                {
                    SHOULD_NOT_REACH_HERE();
                }
            }
            else
            {
                auto *compare = inst->use(0)->def();

                // this instruction is nullcheck and it's known that object (use 0) is not null
                if (is_null_check(compare) && is_not_null(compare->use(0)))
                {
                    // only taken is executable
                    if (!bvisited[br->taken()->id()])
                    {
                        cfg_worklist.push(br->taken());
                    }
                }
                else
                {
                    // both can be executable
                    if (!bvisited[br->taken()->id()])
                    {
                        cfg_worklist.push(br->taken());
                    }

                    if (!bvisited[br->not_taken()->id()])
                    {
                        cfg_worklist.push(br->not_taken());
                    }
                }
            }
        }
        else if (Instruction::isa<Move>(inst))
        {
            auto *def = inst->def();
            auto *use = inst->use(0);

            // propagate data flow info through moves
            bool flag = is_not_null(use);

            if (not_null[def->id()] != flag)
            {
                not_null[def->id()] = flag;
                append_uses_to_worklist(def, ssa_worklist);
            }
        }
    });

    eliminate_null_checks(not_null);
}

void NCE::gather_not_nulls(std::vector<bool> &not_null)
{
    // self is defenetely not null
    not_null[_func->param(0)->id()] = true;

    auto *alloca = _runtime.symbol_by_id(codegen::RuntimeMyIR::GC_ALLOC)->_func;
    for (auto *bb : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        for (auto *inst : bb->insts())
        {
            // results of moves of GlobalConstants are not null
            // results of the allocations are not null
            if ((Instruction::isa<Move>(inst) && Operand::isa<GlobalConstant>(inst->use(0))) ||
                (Instruction::isa<Call>(inst) && Instruction::as<Call>(inst)->callee() == alloca))
            {
                not_null[inst->def()->id()] = true;
            }
        }
    }
}

bool NCE::is_null_check(Instruction *inst)
{
    if (!Instruction::isa<EQ>(inst))
    {
        return false;
    }

    auto *constant = inst->use(1);
    return Operand::isa<Constant>(constant) && Operand::as<Constant>(constant)->value() == 0;
}

void NCE::eliminate_null_checks(std::vector<bool> &not_null)
{
    std::vector<Instruction *> for_elimination;

    for (auto *bb : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        for (auto *inst : bb->insts())
        {
            if (is_null_check(inst))
            {
                auto *use = inst->use(0);
                // check if constant was compared with 0
                if (Operand::isa<GlobalConstant>(use) || not_null[use->id()])
                {
                    for_elimination.push_back(inst);
                }
            }
        }
    }

    for (auto *inst : for_elimination)
    {
        eliminate_null_check(inst);
    }

    merge_blocks();
}

void NCE::eliminate_null_check(Instruction *nullcheck)
{
    auto *pred = nullcheck->def();

    // check if it's really a null check:

    // after <eq x 0> goes 'not'
    if (pred->uses().size() != 1 || !Instruction::isa<Not>(pred->use(0)))
    {
        return;
    }

    // after 'not' goes 'condbr'
    auto *notnull = pred->use(0);
    auto *notnull_def = notnull->def();
    if (notnull->uses().size() != 1 || !Instruction::isa<CondBranch>(notnull_def->use(0)))
    {
        return;
    }

    auto *check_block = nullcheck->holder();
    auto *condbr = Instruction::as<CondBranch>(check_block->insts().back());

    auto *call_block = condbr->taken();
    auto *assert_block = condbr->not_taken();

    auto *merge_block = call_block->succ(0);

    if (merge_block != assert_block->succ(0))
    {
        // TODO: optimize it for CASE
        return;
    }

    // check if call has return value
    auto *maybe_phi = merge_block->insts().front();
    if (Instruction::isa<Phi>(maybe_phi))
    {
        auto *phi = Instruction::as<Phi>(maybe_phi);
        auto *retval = phi->oper_path(call_block);

        auto *def = phi->def();

        // replace phi with move
        merge_block->erase(phi);
        merge_block->append_front(new Move(def, retval));
    }

    Block::disconnect(check_block, assert_block);
    Block::disconnect(assert_block, merge_block);

    auto *not_inst = condbr->use(0)->def();
    auto *compare = not_inst->use(0)->def();

    check_block->erase(condbr);   // delete branch
    check_block->erase(not_inst); // delete not
    check_block->erase(compare);  // delete compare

    check_block->append(new Branch(call_block)); // branch to call
}