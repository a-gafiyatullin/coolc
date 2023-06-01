#include "codegen/arch/myir/emitter/data/DataMyIR.hpp"
#include "codegen/arch/myir/klass/KlassMyIR.hpp"
#include "codegen/arch/myir/symtab/SymbolTableMyIR.hpp"
#include "codegen/emitter/CodeGen.h"
#include <iostream>

namespace codegen
{
class CodeGenMyIR : public CodeGen<myir::oper, Symbol>
{
  private:
    static constexpr std::string_view RUNTIME_MAIN_FUNC = "main";
    static constexpr std::string_view EXT = ".o";

    myir::Module _module;
    myir::IRBuilder _ir_builder;

    const RuntimeMyIR _runtime;
    DataMyIR _data;

    // preallocated operands
    myir::oper const _true_val;
    myir::oper const _false_val;

    myir::oper const _true_obj;
    myir::oper const _false_obj;

    myir::oper const _null_val;

#ifdef DEBUG
    void verify_oop(const myir::oper &object);
#endif // DEBUG

    void add_fields() override;

    void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) override;

    void emit_class_init_method_inner() override;

    inline static myir::oper field_offset(uint64_t offset) { return myir::Constant::constval(myir::UINT64, offset); }

    inline myir::oper pointer_offset(const myir::oper &val)
    {
        static const int POINTER_SIZE_LOG = 3;
        return _ir_builder.shl(val, myir::Constant::constval(myir::UINT32, POINTER_SIZE_LOG));
    }

    myir::oper emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                      const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                     const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_bool_expr(const ast::BoolExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_string_expr(const ast::StringExpression &expr,
                                const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_object_expr_inner(const ast::ObjectExpression &expr,
                                      const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_new_expr_inner(const ast::NewExpression &expr,
                                   const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_cases_expr_inner(const ast::CaseExpression &expr,
                                     const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_let_expr_inner(const ast::LetExpression &expr,
                                   const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_loop_expr_inner(const ast::WhileExpression &expr,
                                    const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_if_expr_inner(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_assign_expr_inner(const ast::AssignExpression &expr,
                                      const std::shared_ptr<ast::Type> &expr_type) override;

    myir::oper emit_in_scope(const std::shared_ptr<ast::ObjectExpression> &object,
                             const std::shared_ptr<ast::Type> &object_type,
                             const std::shared_ptr<ast::Expression> &expr, myir::oper initializer);

    // load/allocate basic values
    myir::oper emit_load_primitive(const myir::oper &obj);
    myir::oper emit_allocate_primitive(const myir::oper &obj, const std::shared_ptr<Klass> &klass);
    myir::oper emit_load_int(const myir::oper &int_obj);
    myir::oper emit_allocate_int(const myir::oper &val);
    myir::oper emit_load_bool(const myir::oper &bool_obj);

    // Main func that allocate Main object and call Main_main
    void emit_runtime_main();

    // helpers
    myir::oper emit_new_inner(const std::shared_ptr<ast::Type> &klass);
    myir::oper emit_new_inner_helper(const std::shared_ptr<ast::Type> &klass, bool preserve_before_init = true);
    myir::oper emit_load_self();
    myir::oper emit_ternary_operator(const myir::oper &pred, const myir::oper &true_val, const myir::oper &false_val);
    void make_control_flow(const myir::oper &pred, myir::block &true_block, myir::block &false_block,
                           myir::block &merge_block);

    // header helpers
    myir::oper emit_load_tag(const myir::oper &obj);
    myir::oper emit_load_size(const myir::oper &obj);
    myir::oper emit_load_dispatch_table(const myir::oper &obj);

  public:
    explicit CodeGenMyIR(const std::shared_ptr<semant::ClassNode> &root);

    void emit(const std::string &out_file) override;
};
}; // namespace codegen