#include "DAE.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

using namespace opt;

char DAE::ID = 0;

bool DAE::runOnFunction(Function &f)
{
    OPT_VERBOSE_ONLY(LOG("DAE: runOnFunction: " + (std::string)f.getName()));

    bool eliminated = false;

    for (auto bb = f.begin(); bb != f.end(); bb++)
    {
        while ((run_on_block(&*bb)))
        {
            eliminated = true;
        }
    }

    return eliminated;
}

bool DAE::run_on_block(BasicBlock *bb)
{
    for (auto inst = bb->begin(); inst != bb->end(); inst++)
    {
        if (auto *memalloc = dyn_cast<CallInst>(inst))
        {
            if (memalloc->getCalledFunction() ==
                _runtime.symbol_by_id(codegen::RuntimeLLVM::RuntimeLLVMSymbols::GC_ALLOC)->_func)
            {
                auto *tag = dyn_cast<ConstantInt>(memalloc->getArgOperand(0));
                if (tag && tag->getValue() == _int_tag)
                {
#ifdef DEBUG
                    print(memalloc, " Found integer allocation in ");
#endif // DEBUG
                    if (can_be_eliminated(memalloc))
                    {
#ifdef DEBUG
                        print(memalloc, "   Allocation can be eliminated in ");
#endif // DEBUG
                        return eliminate(memalloc);
                    }
                }
            }
        }
    }

    return false;
}

bool DAE::can_be_eliminated(const Instruction *inst) const
{
    const Instruction *cast = nullptr;
    if (inst->getNumUses() == 1)
    {
        // should be cast after allocation
        if (!(cast = dyn_cast<BitCastInst>(*inst->user_begin())))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    // two users: getelement ptr and call of constructor
    if (cast->getNumUses() == 2)
    {
        const Value *getelem = nullptr;
        const Value *call = nullptr;
        for (auto user = cast->user_begin(); user != cast->user_end(); user++)
        {
            if (isa<CallInst>(*user))
            {
                call = *user;
            }
            else if (isa<GetElementPtrInst>(*user))
            {
                getelem = *user;
            }
        }

        if (!getelem || !call)
        {
            return false;
        }

        if (getelem->getNumUses() != 1 || !dyn_cast<StoreInst>(*getelem->user_begin()))
        {
            return false;
        }

        // call is right below of the allocation
        if (call == cast->getNextNode())
        {
            return true;
        }
    }

    return false;
}

bool DAE::eliminate(Instruction *inst)
{
    std::vector<Instruction *> insts;

#ifdef LLVM_STATEPOINT_EXAMPLE
    // N instrs before allocations are also dead
#ifdef __aarch64__
    int n = 3; // save sp and fp
#else
    int n = 1; // save sp
#endif
    assert(inst->getPrevNode());
    insts.push_back(inst->getPrevNode());
    for (int i = 0; i < n; i++)
    {
        assert(insts.back()->getPrevNode());
        insts.push_back(insts.back()->getPrevNode());
    }
#endif // LLVM_STATEPOINT_EXAMPLE

    insts.push_back(inst);

    int idx = insts.size() - 1;

    while (idx < insts.size())
    {
        for (auto user = insts[idx]->user_begin(); user != insts[idx]->user_end(); user++)
        {
            insts.push_back(dyn_cast<Instruction>(*user));
        }

        idx++;
    }

#ifdef DEBUG
    for (int i = insts.size() - 1; i >= 0; i--)
    {
        print(insts[i], "   Eliminated instruction from ");
    }
#endif // DEBUG

    for (int i = insts.size() - 1; i >= 0; i--)
    {
        insts[i]->eraseFromParent();
    }

    return true;
}

#ifdef DEBUG
void DAE::print(const Instruction *inst, const std::string &msg)
{
    std::string log;
    llvm::raw_string_ostream llvm_log(log);
    inst->print(llvm_log);

    OPT_VERBOSE_ONLY(LOG(msg + (std::string)inst->getParent()->getName() + ":" + log));
}
#endif // DEBUG