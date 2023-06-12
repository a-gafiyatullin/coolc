#pragma once

#include "codegen/arch/myir/ir/pass/PassManager.hpp"

namespace myir
{
class SSAConstruction : public Pass
{
  private:
    Function *_func; // currently processing function
    CFG *_cfg;

    // for every variable with multiple defs find all blocks that contain it
    std::unordered_map<Operand *, std::set<Block *>> defs_in_blocks() const;

    // standard algorithm for inserting phi-functions
    void insert_phis();

    // renaming algorithm for second phase of SSA construction.
    // Uses standard algorithm with stack for every variable
    void rename_phis(Block *b, std::unordered_map<Variable *, std::stack<Operand *>> &varstacks);

    // φ-function pruning algorithm
    void prune_ssa();
    void prune_ssa_initialize(Block *b, std::unordered_set<Operand *> &alivevars, std::stack<Operand *> &varsstack);
    void prune_ssa_propagate(std::unordered_set<Operand *> &alivevars, std::stack<Operand *> &varsstack);
    void prune_ssa_delete_dead_phis(std::unordered_set<Operand *> &alivevars);

#ifdef DEBUG
    static void dump_defs(const std::unordered_map<Operand *, std::set<Block *>> &defs);
#endif // DEBUG

  public:
    void run(Function *func) override;
};
}; // namespace myir