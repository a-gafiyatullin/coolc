#pragma once

#include "codegen/arch/myir/ir/cfg/CFG.hpp"
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
    void insert_phis(const CFG::DominanceInfo &info);

    // renaming algorithm for second phase of SSA construction.
    // Uses standard algorithm with stack for every variable
    void rename_phis(const CFG::DominanceInfo &info, Block *b,
                     std::unordered_map<Variable *, std::stack<Operand *>> &varstacks);

    // φ-function pruning algorithm
    void prune_ssa(const CFG::DominanceInfo &info);
    void prune_ssa_initialize(const CFG::DominanceInfo &info, Block *b, std::vector<bool> &alivevars,
                              std::stack<Operand *> &varstack);
    void prune_ssa_propagate(std::vector<bool> &alivevars, std::stack<Operand *> &varstack);
    void prune_ssa_delete_dead_phis(const std::vector<bool> &alivevars);

#ifdef DEBUG
    static void dump_defs(const std::unordered_map<Operand *, std::set<Block *>> &defs);
#endif // DEBUG

  public:
    void run(Function *func) override;
};
}; // namespace myir