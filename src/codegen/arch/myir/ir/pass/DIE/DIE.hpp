#pragma once

#include "codegen/arch/myir/ir/pass/PassManager.hpp"

namespace myir
{

// Pass deletes obviously dead instruction
class DIE : public Pass
{
  public:
    void run(Function *func) override;
};

} // namespace myir