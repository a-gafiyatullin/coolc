#pragma once

#include "codegen/arch/myir/ir/IR.hpp"
#include <functional>
#include <stack>

namespace myir
{

class PassManager;

class Pass
{
    friend class PassManager;

  protected:
    Function *_func; // currently processing function
    CFG *_cfg;       // corresponding CFG

    // Uses Algorithm 8.1 from the SSA-based Compiler Design book
    void sparse_data_flow_propagation(
        const std::function<void(Instruction *, std::stack<Instruction *> &ssa_worklist,
                                 std::stack<Block *> &cfg_worklist, std::vector<bool> &bvisited)> &visitor);

    // helpers
    std::vector<Operand *> operands_from_executable_paths(Instruction *inst, std::vector<bool> &bvisited);
    void append_uses_to_worklist(Operand *operand, std::stack<Instruction *> &ssa_worklist);

    // usefull post passes
    void merge_blocks();

  public:
    virtual void run(Function *func) = 0;

    virtual ~Pass() {}
};

class PassManager : allocator::StackObject
{
  private:
    std::vector<Pass *> _passes;
    Module &_module;

  public:
    PassManager(Module &module) : _module(module) {}

    inline void add(Pass *pass) { _passes.push_back(pass); }

    void run();

    ~PassManager();
};
}; // namespace myir