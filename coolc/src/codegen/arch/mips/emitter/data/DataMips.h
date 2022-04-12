#pragma once

#include "codegen/arch/mips/klass/KlassMips.h"
#include "codegen/arch/mips/runtime/RuntimeMips.h"
#include "codegen/emitter/data/Data.h"
#include "codegen/symnames/NameConstructor.h"
#include <cmath>

namespace codegen
{
/**
 * @brief Maintain data section
 *
 */
class DataMips : public Data<const Label &>
{
  private:
    CodeBuffer _code; // final code
    Assembler _asm;   // main assembler

    const RuntimeMips &_runtime;

    // gather code
    void gen_prototypes();
    void gen_dispatch_tabs();

    void gen_class_obj_tab() override;
    void gen_class_name_tab() override;

    void string_const_inner(const std::string &str) override;
    void bool_const_inner(const bool &value) override;
    void int_const_inner(const int64_t &value) override;
    void class_struct_inner(const std::shared_ptr<Klass> &klass) override;
    void class_disp_tab_inner(const std::shared_ptr<Klass> &klass) override;

    void emit_inner(const std::string &out_file) override;

  public:
    /**
     * @brief Construct a new DataMips object
     *
     * @param builder KlassBuilder
     * @param runtime Runtime declarations
     */
    DataMips(const std::shared_ptr<KlassBuilder> &builder, const RuntimeMips &runtime);
};
}; // namespace codegen