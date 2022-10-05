#include "codegen/arch/llvm/emitter/data/DataLLVM.h"
#include "codegen/arch/llvm/klass/KlassLLVM.h"
#include "codegen/arch/llvm/symtab/SymbolTableLLVM.h"
#include "codegen/emitter/CodeGen.h"
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <filesystem>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

namespace codegen
{
class CodeGenLLVM : public CodeGen<llvm::Value *, Symbol>
{
  private:
    static constexpr std::string_view RUNTIME_MAIN_FUNC = "main";
    static constexpr std::string_view EXT = ".o";
    static constexpr std::string_view RUNTIME_LIB_NAME = "libcool-rt.so";
    static constexpr std::string_view CLANG_EXE_NAME = "clang++";

#ifdef LLVM_STATEPOINT_EXAMPLE
    static constexpr std::string_view OBJCOPY_EXE_NAME = "objcopy";
    static constexpr std::string_view STACKMAP_NAME = "__LLVM_StackMaps";
#endif // LLVM_STATEPOINT_EXAMPLE

    // llvm related stuff
    llvm::LLVMContext _context;
    llvm::IRBuilder<> _ir_builder;
    llvm::Module _module;

    const RuntimeLLVM _runtime;
    DataLLVM _data;

    // optimizations
    llvm::legacy::FunctionPassManager _optimizer;
    void init_optimizer();

    // helper values
    llvm::Value *const _true_obj;
    llvm::Value *const _false_obj;

    llvm::Value *const _true_val;
    llvm::Value *const _false_val;

    llvm::Value *const _int0_64;
    llvm::Value *const _int0_32;
    llvm::Value *const _int0_8;

    llvm::Value *const _int0_8_ptr;
    llvm::Value *const _stack_slot_null;

    // stack
    std::vector<llvm::Value *> _stack;
    int _current_stack_size;
#ifdef DEBUG
    int _max_stack_size;
#endif // DEBUG

    void add_fields() override;

    void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) override;

    void emit_class_init_method_inner() override;

#ifdef LLVM_SHADOW_STACK
    void allocate_shadow_stack(int max_stack);
    void init_shadow_stack(const std::vector<llvm::Value *> &args);

    int preserve_value_for_gc(llvm::Value *value, bool preserve);
    void pop_dead_value(int slots = 1);
    llvm::Value *reload_value_from_stack(int slot_num, llvm::Value *orig_value, bool reload);

    int reload_args(std::vector<llvm::Value *> &args, const std::shared_ptr<ast::Expression> &self,
                    const std::vector<std::shared_ptr<ast::Expression>> &expr_args, int slots);

    bool _need_reload;
    void set_need_reload(bool need_reload);
#else
    void allocate_stack(int max_stack);
    void init_stack();
    void push(llvm::Value *value);
    void pop();
#endif // LLVM_SHADOW_STACK

#ifdef LLVM_STATEPOINT_EXAMPLE
    void save_frame();
#endif // LLVM_STATEPOINT_EXAMPLE

    // cast values helpers
    llvm::Value *maybe_cast(llvm::Value *val, llvm::Type *type);
    void maybe_cast(std::vector<llvm::Value *> &args, llvm::FunctionType *func);

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
    llvm::Value *emit_new_inner_helper(const std::shared_ptr<ast::Type> &klass, bool preserve_before_init = true);
    llvm::Value *emit_load_self();
    llvm::Value *emit_ternary_operator(llvm::Value *pred, llvm::Value *true_val, llvm::Value *false_val);
    void make_control_flow(llvm::Value *pred, llvm::BasicBlock *&true_block, llvm::BasicBlock *&false_block,
                           llvm::BasicBlock *&merge_block);

    // header helpers
    llvm::Value *emit_load_tag(llvm::Value *obj, llvm::Type *obj_type);
    llvm::Value *emit_load_size(llvm::Value *objv, llvm::Type *obj_type);
    llvm::Value *emit_load_dispatch_table(llvm::Value *obj, const std::shared_ptr<Klass> &klass);

    void execute_linker(const std::string &object_file_name, const std::string &out_file_name);
    std::pair<std::string, std::string> find_best_vec_ext();

#ifdef DEBUG
    void verify(llvm::Function *func);
#endif // DEBUG
  public:
    explicit CodeGenLLVM(const std::shared_ptr<semant::ClassNode> &root);

    void emit(const std::string &out_file) override;
};
}; // namespace codegen