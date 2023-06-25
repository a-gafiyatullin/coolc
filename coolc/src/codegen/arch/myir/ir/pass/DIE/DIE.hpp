#pragma once

#include "codegen/arch/myir/ir/pass/PassManager.hpp"

namespace myir
{

// pass deletes obviously dead instruction
class DIE : public Pass
{
  public:
    void run(Function *func) override;
};

} // namespace myir