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
        if (Instruction::isa<Phi>(inst))
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

            // get optimisitic prediction
            bool not_null_state = true;
            for (auto *o : executable)
            {
                not_null_state &= not_null[o->id()];
            }
            not_null[phi->def()->id()] = not_null_state;
        }
        else if (Instruction::isa<CondBranch>(inst))
        {
            // use the best estimaton for this branch
            auto *br = Instruction::as<CondBranch>(inst);
            auto *compare = inst->use(0)->def();

            // this instruction is nullcheck and it's known that object (use 0) is not null
            if (is_null_check(compare) && not_null[compare->use(0)->id()])
            {
                // only taken is executable
                cfg_worklist.push(br->taken());
            }
            else
            {
                // both can be executable
                cfg_worklist.push(br->taken());
                cfg_worklist.push(br->not_taken());
            }
        }
        else if (Instruction::isa<Move>(inst))
        {
            // propagate data flow info through moves
            if (!not_null[inst->def()->id()])
            {
                not_null[inst->def()->id()] = not_null[inst->use(0)->id()];
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
                if (not_null[inst->use(0)->id()])
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
    assert(merge_block == assert_block->succ(0));

    // check if call has return value
    auto *maybe_phi = merge_block->insts().front();
    if (Instruction::isa<Phi>(maybe_phi))
    {
        auto *phi = Instruction::as<Phi>(maybe_phi);
        auto *retval = phi->oper_path(call_block);

        auto *def = phi->def();

        // replace phi with move
        merge_block->erase(phi);
        merge_block->append_front(new Move(def, retval, merge_block));
    }

    Block::disconnect(check_block, assert_block);
    Block::disconnect(assert_block, merge_block);

    auto *not_inst = condbr->use(0)->def();
    auto *compare = not_inst->use(0)->def();

    check_block->erase(condbr);   // delete branch
    check_block->erase(not_inst); // delete not
    check_block->erase(compare);  // delete compare

    check_block->append(new Branch(call_block, check_block)); // branch to call
}