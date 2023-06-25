#pragma once

#include "codegen/arch/myir/ir/pass/PassManager.hpp"
#include <stack>

namespace myir
{

// this pass:
// 1. eliminates unnecessary loads from primitives
// 2. creates object wrappers if object escapes to call / return
class Unboxing : public Pass
{
  private:
    void replace_args(std::vector<bool> &processed);
    void replace_lets(std::vector<bool> &processed);

    // replace all uses with a new def and fix instructions
    void replace_uses(std::stack<Instruction *> &s, std::vector<bool> &processed);
    void replace_load(Load *load, std::stack<Instruction *> &s);
    void replace_store(Store *store, std::stack<Instruction *> &s);

  public:
    void run(Function *func) override;
};
}; // namespace myir