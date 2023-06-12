#pragma once

#include "codegen/arch/myir/ir/IR.hpp"

namespace myir
{

class Pass
{
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