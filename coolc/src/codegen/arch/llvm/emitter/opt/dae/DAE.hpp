#pragma once

#include "codegen/arch/llvm/runtime/RuntimeLLVM.h"
#include <llvm/Pass.h>

using namespace llvm;

namespace opt
{

/**
 * @brief Dead Allocation Elimination
 *
 */
struct DAE : public FunctionPass
{
    static char ID;

    const codegen::RuntimeLLVM &_runtime;

    /**
     * @brief Construct a LVN Pass
     *
     * @param rt Runtime methods
     */
    DAE(const codegen::RuntimeLLVM &rt, int int_tag) : FunctionPass(ID), _runtime(rt), _int_tag(int_tag)
    {
    }

    bool runOnFunction(Function &f) override;

  private:
    int _int_tag;

#ifdef DEBUG
    void print(const Instruction *inst, const std::string &msg);
#endif // DEBUG

    bool run_on_block(BasicBlock *bb);

    bool can_be_eliminated(const Instruction *inst) const;
    bool eliminate(Instruction *inst);
};
} // namespace opt