#include "codegen/arch/myir/emitter/data/DataMyIR.hpp"
#include "codegen/arch/myir/klass/KlassMyIR.hpp"
#include "codegen/arch/myir/symtab/SymbolTableMyIR.hpp"
#include "codegen/emitter/CodeGen.h"
#include <iostream>

namespace codegen
{
class CodeGenMyIR : public CodeGen<myir::Operand *, Symbol>
{
  private:
    static constexpr std::string_view RUNTIME_MAIN_FUNC = "main";
    static constexpr std::string_view EXT = ".o";

    myir::Module _module;
    myir::IRBuilder _ir_builder;

    const RuntimeMyIR _runtime;
    DataMyIR _data;

    // preallocated operands
    myir::Operand *const _true_val;
    myir::Operand *const _false_val;

    myir::Operand *const _true_obj;
    myir::Operand *const _false_obj;

    myir::Operand *const _null_val;

#ifdef DEBUG
    void verify_oop(myir::Operand *object);
#endif // DEBUG

    void add_fields() override;

    void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) override;

    void emit_class_init_method_inner() override;

    inline static myir::Operand *field_offset(uint64_t offset) { return new myir::Constant(offset, myir::UINT64); }

    inline myir::Operand *pointer_offset(myir::Operand *val)
    {
        static const int POINTER_SIZE_LOG = 3;
        return _ir_builder.shl(val, new myir::Constant(POINTER_SIZE_LOG, myir::UINT32));
    }

    myir::Operand *emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                          const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                         const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_bool_expr(const ast::BoolExpression &expr,
                                  const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_string_expr(const ast::StringExpression &expr,
                                    const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_object_expr_inner(const ast::ObjectExpression &expr,
                                          const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_new_expr_inner(const ast::NewExpression &expr,
                                       const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_cases_expr_inner(const ast::CaseExpression &expr,
                                         const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_let_expr_inner(const ast::LetExpression &expr,
                                       const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_loop_expr_inner(const ast::WhileExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_if_expr_inner(const ast::IfExpression &expr,
                                      const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                            const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_assign_expr_inner(const ast::AssignExpression &expr,
                                          const std::shared_ptr<ast::Type> &expr_type) override;

    myir::Operand *emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                                 const std::shared_ptr<ast::Type> &object_type,
                                 const std::shared_ptr<ast::Expression> &expr, myir::Operand *initializer);

    // load/allocate basic values
    myir::Operand *emit_load_primitive(myir::Operand *obj);
    myir::Operand *emit_allocate_primitive(myir::Operand *obj, const std::shared_ptr<Klass> &klass);
    myir::Operand *emit_load_int(myir::Operand *int_obj);
    myir::Operand *emit_allocate_int(myir::Operand *val);
    myir::Operand *emit_load_bool(myir::Operand *bool_obj);

    // Main func that allocate Main object and call Main_main
    void emit_runtime_main();

    // helpers
    myir::Operand *emit_new_inner(const std::shared_ptr<ast::Type> &klass);
    myir::Operand *emit_new_inner_helper(const std::shared_ptr<ast::Type> &klass, bool preserve_before_init = true);
    myir::Operand *emit_load_self();
    myir::Operand *emit_ternary_operator(myir::Operand *pred, myir::Operand *true_val, myir::Operand *false_val);
    void make_control_flow(myir::Operand *pred, myir::Block *&true_block, myir::Block *&false_block,
                           myir::Block *&merge_block);

    // header helpers
    myir::Operand *emit_load_tag(myir::Operand *obj);
    myir::Operand *emit_load_size(myir::Operand *obj);
    myir::Operand *emit_load_dispatch_table(myir::Operand *obj);

  public:
    explicit CodeGenMyIR(const std::shared_ptr<semant::ClassNode> &root);

    void emit(const std::string &out_file) override;
};
}; // namespace codegen