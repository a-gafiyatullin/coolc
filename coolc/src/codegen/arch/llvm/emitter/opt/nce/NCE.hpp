#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include <llvm/IR/Instruction.h>

using namespace llvm;

struct NCE : public FunctionPass
{
    static char ID;

    NCE() : FunctionPass(ID)
    {
    }

    bool runOnFunction(Function &f) override;

    /**
     * @brief Search for null check instructions
     *
     * @param bb Basic block to inspect
     * @return Instruction* Null Check Instruction
     */
    Instruction *null_check(BasicBlock *bb);

    /**
     * @brief Check if null check can be eliminated
     *
     * @param nce_inst Null check instruction
     * @return true Can be eliminated
     * @return false Cannot be eliminated
     */
    bool can_be_eliminated(Instruction *nce_inst);

    /**
     * @brief Try to eliminate null check
     *
     * @param nce_inst Null check instruction
     * @return true Was eliminated
     * @return false Was not eliminated
     */
    bool eliminate_null_check(Instruction *nce_inst);
};