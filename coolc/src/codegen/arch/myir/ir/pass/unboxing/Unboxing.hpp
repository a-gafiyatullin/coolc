#pragma once

#include "codegen/arch/myir/emitter/data/DataMyIR.hpp"
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
    struct TypeLink
    {
        Instruction *_inst;
        OperandType _type; // original type of the object that was replaced
    };

    const codegen::RuntimeMyIR &_runtime;
    const std::shared_ptr<codegen::KlassBuilder> &_builder;
    codegen::DataMyIR &_data;
    Module &_module;

    void replace_args(std::vector<bool> &processed);
    void replace_lets(std::vector<bool> &processed);

    // replace all uses with a new def and fix instructions
    void replace_uses(std::stack<TypeLink> &s, std::vector<bool> &processed);
    void replace_load(Load *load, OperandType type, std::stack<TypeLink> &s);
    void replace_store(Store *store, OperandType type, std::stack<TypeLink> &s);
    void wrap_primitives(Instruction *inst, OperandType type);

    // this function mostly copies logic of the CodeGenMyIR::emit_allocate_primitive but inserts a new code after
    // specific instruction
    Operand *allocate_primitive(Instruction *before, Operand *value, const std::shared_ptr<ast::Type> &klass_type);
    std::shared_ptr<codegen::Klass> operand_to_klass(OperandType type);

  public:
    Unboxing(const codegen::RuntimeMyIR &runtime, codegen::DataMyIR &data,
             const std::shared_ptr<codegen::KlassBuilder> &builder, Module &module)
        : _runtime(runtime), _data(data), _builder(builder), _module(module)
    {
    }

    void run(Function *func) override;
};
}; // namespace myir