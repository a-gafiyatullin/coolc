#pragma once

#include "semant/Semant.h"

namespace codegen
{

/**
 * @brief CodeGen emits code for methods
 *
 */
class CodeGen
{
  protected:
    // current generating class
    std::shared_ptr<ast::Class> _current_class;

    // program ast
    std::shared_ptr<semant::ClassNode> _root;

    // emit code for class, add fields to symbol table
    void emit_class_code(const std::shared_ptr<semant::ClassNode> &node);
    virtual void emit_class_code_inner(const std::shared_ptr<semant::ClassNode> &node) = 0;

    // methods of class
    void emit_class_method(const std::shared_ptr<ast::Feature> &method);
    virtual void emit_class_method_inner(const std::shared_ptr<ast::Feature> &method) = 0;

    void emit_class_init_method(const std::shared_ptr<ast::Class> &klass);
    virtual void emit_class_init_method_inner(const std::shared_ptr<ast::Class> &klass) = 0;

    // emit expressions
    void emit_expr(const std::shared_ptr<ast::Expression> &expr);

    void emit_binary_expr(const ast::BinaryExpression &expr);
    virtual void emit_binary_expr_inner(const ast::BinaryExpression &expr) = 0;

    void emit_unary_expr(const ast::UnaryExpression &expr);
    virtual void emit_unary_expr_inner(const ast::UnaryExpression &expr) = 0;

    virtual void emit_bool_expr(const ast::BoolExpression &expr) = 0;
    virtual void emit_int_expr(const ast::IntExpression &expr) = 0;
    virtual void emit_string_expr(const ast::StringExpression &expr) = 0;

    void emit_object_expr(const ast::ObjectExpression &expr);
    virtual void emit_object_expr_inner(const ast::ObjectExpression &expr) = 0;

    void emit_new_expr(const ast::NewExpression &expr);
    virtual void emit_new_expr_inner(const ast::NewExpression &expr) = 0;

    void emit_cases_expr(const ast::CaseExpression &expr);
    virtual void emit_cases_expr_inner(const ast::CaseExpression &expr) = 0;

    void emit_let_expr(const ast::LetExpression &expr);
    virtual void emit_let_expr_inner(const ast::LetExpression &expr) = 0;

    void emit_list_expr(const ast::ListExpression &expr);
    virtual void emit_list_expr_inner(const ast::ListExpression &expr) = 0;

    void emit_loop_expr(const ast::WhileExpression &expr);
    virtual void emit_loop_expr_inner(const ast::WhileExpression &expr) = 0;

    void emit_if_expr(const ast::IfExpression &expr);
    virtual void emit_if_expr_inner(const ast::IfExpression &expr) = 0;

    void emit_dispatch_expr(const ast::DispatchExpression &expr);
    virtual void emit_dispatch_expr_inner(const ast::DispatchExpression &expr) = 0;

    void emit_assign_expr(const ast::AssignExpression &expr);
    virtual void emit_assign_expr_inner(const ast::AssignExpression &expr) = 0;

  public:
    /**
     * @brief Construct a new CodeGen object
     *
     * @param root Root of program class hierarhy
     */
    CodeGen(const std::shared_ptr<semant::ClassNode> &root);

    /**
     * @brief Emit code
     *
     * @param out_file Result file name
     */
    virtual void emit(const std::string &out_file) = 0;
};

}; // namespace codegen