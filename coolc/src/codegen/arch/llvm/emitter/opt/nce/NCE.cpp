#include "NCE.hpp"
#include "utils/Utils.h"
#include <llvm/IR/CFG.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

char NCE::ID = 0;

bool NCE::runOnFunction(Function &f)
{
    OPT_VERBOSE_ONLY(LOG("NCE: runOnFunction: " + (std::string)f.getName()));

    for (auto bb = f.begin(); bb != f.end(); bb++)
    {
        if (auto *nce_inst = null_check(&*bb))
        {

#ifdef DEBUG
            std::string log;
            llvm::raw_string_ostream llvm_log(log);
            nce_inst->print(llvm_log);

            OPT_VERBOSE_ONLY(LOG(" Found null check inst for " + (std::string)bb->getName() + ":" + log));
#endif // DEBUG

            if (can_be_eliminated(nce_inst))
            {
                OPT_VERBOSE_ONLY(LOG("   Can eliminate this null check."));

                return eliminate_null_check(nce_inst);
            }
        }
    }

    return false;
}

Instruction *NCE::null_check(BasicBlock *bb)
{
    for (auto inst = bb->rbegin(); inst != bb->rend(); inst++)
    {
        if (isa<ICmpInst>(*inst))
        {
            bool has_null_use = false;

            for (const auto *use = inst->op_begin(); use != inst->op_end(); use++)
            {
                if (isa<llvm::ConstantPointerNull>(*use))
                {
                    return &*inst;
                }
            }
        }
    }

    return nullptr;
}

bool NCE::can_be_eliminated(Instruction *nce_inst)
{
    // if value that we check is:
    // 1) self
    // 2) result of allocation
    // we can eliminate null check

    auto *const self = nce_inst->getFunction()->getArg(0);

    // find value to be checked
    Value *val = nullptr;

    for (const auto *use = nce_inst->op_begin(); use != nce_inst->op_end(); use++)
    {
        if (!isa<llvm::ConstantPointerNull>(*use))
        {
            val = use->get();
            break;
        }
    }
    assert(val);

    // 1. Val is load. It's ok if corresponding slot is slot for self
    if (const auto *load = dyn_cast<LoadInst>(val))
    {
        auto *const slot = load->getPointerOperand();
        if (const auto *const alloca = dyn_cast<AllocaInst>(slot))
        {
            int store_cnt = 0;
            bool store_self = false;

            for (auto user = alloca->user_begin(); user != alloca->user_end(); user++)
            {
                if (const auto *const store = dyn_cast<StoreInst>(*user))
                {
                    store_cnt++;

                    if (store->getValueOperand() == self)
                    {
                        store_self = true;
                    }
                }
            }

            // self object can be stored only one time
            if (store_cnt == 1 && store_self)
            {
                return true;
            }

            assert(!store_self);
        }
    }

    return false;
}

bool NCE::eliminate_null_check(Instruction *nce_inst)
{
    auto *branch = dyn_cast<BranchInst>(nce_inst->getNextNode());
    assert(branch);

    auto *check_block = nce_inst->getParent();

    auto *true_block = branch->getSuccessor(0);
    auto *false_block = branch->getSuccessor(1);

    auto *merge_block = true_block->getSingleSuccessor();

    // case expression. Don't process for now
    if (!merge_block || merge_block != false_block->getSinglePredecessor())
    {
        OPT_VERBOSE_ONLY(LOG("   Cannot eliminate null check for case expression!"));

        return false;
    }

    // 1. delete last two insts from check block
    nce_inst->eraseFromParent();
    branch->eraseFromParent();

    // 2. delete false block and last inst from the true block
    false_block->eraseFromParent();
    assert(isa<BranchInst>(true_block->back()));
    true_block->back().eraseFromParent();

    // 3. delete phi (first inst) in the merge block
    assert(isa<PHINode>(merge_block->front()));
    assert(isa<CallInst>(true_block->back()));
    merge_block->front().replaceAllUsesWith(&true_block->back());
    merge_block->front().eraseFromParent();

    // 4. merge blocks
    check_block->getInstList().splice(check_block->end(), true_block->getInstList());
    check_block->getInstList().splice(check_block->end(), merge_block->getInstList());

    true_block->eraseFromParent();
    merge_block->eraseFromParent();

    return true;
}