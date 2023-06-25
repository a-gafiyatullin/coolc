#pragma once

#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"
#include "codegen/emitter/data/Data.h"

namespace codegen
{

class DataMyIR : public Data<myir::Operand *>
{
  private:
    myir::Module &_module;
    const RuntimeMyIR &_runtime;

    void string_const_inner(const std::string &str) override;
    void bool_const_inner(const bool &value) override;
    void int_const_inner(const int64_t &value) override;

    void class_struct_inner(const std::shared_ptr<Klass> &klass) override {}

    void class_disp_tab_inner(const std::shared_ptr<Klass> &klass) override;

    void gen_class_obj_tab_inner();
    void gen_class_obj_tab() override {}

    void gen_class_name_tab() override;

    void make_init_method(const std::shared_ptr<Klass> &klass);

    void emit_inner(const std::string &out_file) override;

  public:
    /**
     * @brief Construct a new Data manager for MyIR
     *
     * @param builder Klasses
     * @param module MyIR module
     * @param runtime Runtime methods
     */
    DataMyIR(const std::shared_ptr<KlassBuilder> &builder, myir::Module &module, const RuntimeMyIR &runtime);

    myir::OperandType ast_to_ir_type(const std::shared_ptr<ast::Type> &type);
};

}; // namespace codegen