#pragma once

#include "codegen/arch/myir/ir/Oper.hpp"
#include "codegen/arch/myir/ir/pass/PassManager.hpp"

namespace myir
{

// Copy propagation
class CP : public Pass
{
  private:
    void eliminate_copies(std::unordered_map<Operand *, Operand *> &copies);

  public:
    void run(Function *func) override;
};

} // namespace myir
