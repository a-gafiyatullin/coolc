#pragma once

#include "codegen/arch/llvm/runtime/RuntimeLLVM.h"
#include <llvm/Pass.h>

using namespace llvm;

namespace opt
{

/**
 * @brief Null Check Elimination
 *
 */
struct NCE : public FunctionPass
{
    static char ID;

    const codegen::RuntimeLLVM &_runtime;

    /**
     * @brief Construct a NCE Pass
     *
     * @param rt Runtime methods
     */
    NCE(const codegen::RuntimeLLVM &rt) : FunctionPass(ID), _runtime(rt) {}

    bool runOnFunction(Function &f) override;

  private:
    Instruction *null_check(BasicBlock *bb);
    bool can_be_eliminated(Instruction *nce_inst);
    bool eliminate_null_check(Instruction *nce_inst);
};
} // namespace opt
