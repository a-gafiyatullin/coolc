#pragma once

#include "codegen/klass/Klass.h"
#include "codegen/symtab/SymbolTable.h"

namespace codegen
{

/**
 * @brief CodeGen emits code for methods
 *
 */
template <class Value, class Symbol> class CodeGen
{
  protected:
    // current generating class
    std::shared_ptr<ast::Class> _current_class;

    // symbol table
    SymbolTable<Symbol> _table;

    // Klasses
    const std::shared_ptr<KlassBuilder> _builder;

    // emit code for class, add fields to symbol table
    void emit_class_code(const std::shared_ptr<semant::ClassNode> &node);

    // emit_class_code helpers
    virtual void add_fields() = 0;

    // methods of class
    void emit_class_method(const std::shared_ptr<ast::Feature> &method);
    virtual void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) = 0;

    void emit_class_init_method();
    virtual void emit_class_init_method_inner() = 0;

    // emit expressions
    Value emit_expr(const std::shared_ptr<ast::Expression> &expr);

    Value emit_binary_expr(const ast::BinaryExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_binary_expr_inner(const ast::BinaryExpression &expr,
                                         const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_unary_expr(const ast::UnaryExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_unary_expr_inner(const ast::UnaryExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) = 0;

    virtual Value emit_bool_expr(const ast::BoolExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;
    virtual Value emit_int_expr(const ast::IntExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;
    virtual Value emit_string_expr(const ast::StringExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_object_expr(const ast::ObjectExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_object_expr_inner(const ast::ObjectExpression &expr,
                                         const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_new_expr(const ast::NewExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_new_expr_inner(const ast::NewExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_cases_expr(const ast::CaseExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_cases_expr_inner(const ast::CaseExpression &expr,
                                        const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_let_expr(const ast::LetExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_let_expr_inner(const ast::LetExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_list_expr(const ast::ListExpression &expr, const std::shared_ptr<ast::Type> &expr_type);

    Value emit_loop_expr(const ast::WhileExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_loop_expr_inner(const ast::WhileExpression &expr,
                                       const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_if_expr(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_if_expr_inner(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_dispatch_expr(const ast::DispatchExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_dispatch_expr_inner(const ast::DispatchExpression &expr,
                                           const std::shared_ptr<ast::Type> &expr_type) = 0;

    Value emit_assign_expr(const ast::AssignExpression &expr, const std::shared_ptr<ast::Type> &expr_type);
    virtual Value emit_assign_expr_inner(const ast::AssignExpression &expr,
                                         const std::shared_ptr<ast::Type> &expr_type) = 0;

  public:
    /**
     * @brief Construct a new CodeGen object
     *
     * @param data Klass builder
     */
    explicit CodeGen(const std::shared_ptr<KlassBuilder> &builder);

    /**
     * @brief Emit code
     *
     * @param out_file Result file name
     */
    virtual void emit(const std::string &out_file) = 0;
};

}; // namespace codegen