#include "codegen/arch/llvm/emitter/data/DataLLVM.h"
#include "codegen/arch/llvm/klass/KlassLLVM.h"
#include "codegen/arch/llvm/symtab/SymbolTableLLVM.h"
#include "codegen/emitter/CodeGen.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

namespace codegen
{
class CodeGenLLVM : public CodeGen<llvm::Value *, Symbol>
{
  private:
    static constexpr std::string_view RUNTIME_MAIN_FUNC = "main";
    static constexpr std::string_view EXT = ".o";

    // llvm related stuff
    llvm::LLVMContext _context;
    llvm::IRBuilder<> _ir_builder;
    llvm::Module _module;

    const RuntimeLLVM _runtime;
    DataLLVM _data;

    // helper values
    llvm::Value *_true_obj;
    llvm::Value *_false_obj;

    llvm::Value *_true_val;
    llvm::Value *_false_val;

    void add_fields() override;

    void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) override;

    void emit_class_init_method_inner() override;

    llvm::Value *emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                       const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_bool_expr(const ast::BoolExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_string_expr(const ast::StringExpression &expr,
                                  const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_object_expr_inner(const ast::ObjectExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_new_expr_inner(const ast::NewExpression &expr,
                                     const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_cases_expr_inner(const ast::CaseExpression &expr,
                                       const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_let_expr_inner(const ast::LetExpression &expr,
                                     const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_loop_expr_inner(const ast::WhileExpression &expr,
                                      const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_if_expr_inner(const ast::IfExpression &expr,
                                    const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                          const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_assign_expr_inner(const ast::AssignExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) override;

    llvm::Value *emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                               const std::shared_ptr<ast::Type> &object_type,
                               const std::shared_ptr<ast::Expression> &expr, llvm::Value *initializer);

    // load/allocate basic values
    llvm::Value *emit_load_primitive(llvm::Value *obj, llvm::Type *obj_type);
    llvm::Value *emit_allocate_primitive(llvm::Value *obj, const std::shared_ptr<Klass> &klass);
    llvm::Value *emit_load_int(llvm::Value *int_obj);
    llvm::Value *emit_allocate_int(llvm::Value *val);
    llvm::Value *emit_load_bool(llvm::Value *bool_obj);

    // void emit_gc_update(const Register &obj, const int &offset);

    // Main func that allocate Main object and call Main_main
    void emit_runtime_main();

    // helpers
    llvm::Value *emit_new_inner(const std::shared_ptr<ast::Type> &klass);
    llvm::Value *emit_load_self();
    llvm::Value *emit_ternary_operator(llvm::Value *pred, llvm::Value *true_val, llvm::Value *false_val,
                                       llvm::Type *type);

    // header helpers
    llvm::Value *emit_load_tag(llvm::Value *obj, llvm::Type *obj_type);
    llvm::Value *emit_load_size(llvm::Value *objv, llvm::Type *obj_type);
    llvm::Value *emit_load_dispatch_table(llvm::Value *obj, const std::shared_ptr<Klass> &klass);

  public:
    explicit CodeGenLLVM(const std::shared_ptr<semant::ClassNode> &root);

    void emit(const std::string &out_file) override;
};
}; // namespace codegen