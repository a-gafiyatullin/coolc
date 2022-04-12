#pragma once

#include "codegen/arch/llvm/klass/KlassLLVM.h"
#include "codegen/arch/llvm/runtime/RuntimeLLVM.h"
#include "codegen/emitter/data/Data.h"
#include "codegen/symnames/NameConstructor.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

namespace codegen
{

class DataLLVM : public Data<llvm::Constant *, llvm::StructType *, llvm::Constant *>
{
  private:
    static constexpr std::string_view TYPE_SUFFIX = "_type";

    enum HeaderLayout
    {
        Mark,
        Tag,
        Size,
        DispatchTable
    };

    llvm::Module &_module;
    const RuntimeLLVM &_runtime;

    void string_const_inner(const std::string &str) override;
    void bool_const_inner(const bool &value) override;
    void int_const_inner(const int64_t &value) override;
    void class_struct_inner(const std::shared_ptr<Klass> &klass) override;
    void class_disp_tab_inner(const std::shared_ptr<Klass> &klass) override;

    void gen_class_obj_tab() override;
    void gen_class_name_tab() override;

    void emit_inner(const std::string &out_file) override;

    // helpers
    void construct_header(const std::shared_ptr<Klass> &klass, std::vector<llvm::Type *> &fields);
    void construct_base_class(const std::shared_ptr<Klass> &klass, const std::vector<llvm::Type *> &fields,
                              const std::vector<llvm::Constant *> &methods);

  public:
    /**
     * @brief Construct a new Data manager for LLVM
     *
     * @param builder Klasses
     * @param module LLVM module
     * @param runtime Runtime methods
     */
    DataLLVM(const std::shared_ptr<KlassBuilder> &builder, llvm::Module &module, const RuntimeLLVM &runtime);
};

}; // namespace codegen
