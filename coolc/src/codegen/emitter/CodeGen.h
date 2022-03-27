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

    Value emit_binary_expr(const ast::BinaryExpression &expr);
    virtual Value emit_binary_expr_inner(const ast::BinaryExpression &expr) = 0;

    Value emit_unary_expr(const ast::UnaryExpression &expr);
    virtual Value emit_unary_expr_inner(const ast::UnaryExpression &expr) = 0;

    virtual Value emit_bool_expr(const ast::BoolExpression &expr) = 0;
    virtual Value emit_int_expr(const ast::IntExpression &expr) = 0;
    virtual Value emit_string_expr(const ast::StringExpression &expr) = 0;

    Value emit_object_expr(const ast::ObjectExpression &expr);
    virtual Value emit_object_expr_inner(const ast::ObjectExpression &expr) = 0;

    Value emit_new_expr(const ast::NewExpression &expr);
    virtual Value emit_new_expr_inner(const ast::NewExpression &expr) = 0;

    Value emit_cases_expr(const ast::CaseExpression &expr);
    virtual Value emit_cases_expr_inner(const ast::CaseExpression &expr) = 0;

    Value emit_let_expr(const ast::LetExpression &expr);
    virtual Value emit_let_expr_inner(const ast::LetExpression &expr) = 0;

    Value emit_list_expr(const ast::ListExpression &expr);

    Value emit_loop_expr(const ast::WhileExpression &expr);
    virtual Value emit_loop_expr_inner(const ast::WhileExpression &expr) = 0;

    Value emit_if_expr(const ast::IfExpression &expr);
    virtual Value emit_if_expr_inner(const ast::IfExpression &expr) = 0;

    Value emit_dispatch_expr(const ast::DispatchExpression &expr);
    virtual Value emit_dispatch_expr_inner(const ast::DispatchExpression &expr) = 0;

    Value emit_assign_expr(const ast::AssignExpression &expr);
    virtual Value emit_assign_expr_inner(const ast::AssignExpression &expr) = 0;

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