#pragma once

#include "codegen/arch/myir/ir/pass/PassManager.hpp"
#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"

namespace myir
{

// Pass deletes null checks
class NCE : public Pass
{
  private:
    const codegen::RuntimeMyIR &_runtime;

    // find all not null defs in the method
    void gather_not_nulls(std::vector<bool> &not_null);

    // delete unnecessary null checks
    void eliminate_null_checks(std::vector<bool> &not_null);
    void eliminate_null_check(Instruction *nullcheck);

    // check if instruction is <eq x 0>
    bool is_null_check(Instruction *inst);

  public:
    void run(Function *func) override;

    NCE(const codegen::RuntimeMyIR &rt) : _runtime(rt) {}
};

} // namespace myir
