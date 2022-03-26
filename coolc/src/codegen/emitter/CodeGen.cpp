#include "codegen/emitter/CodeGen.h"

using namespace codegen;

CodeGen::CodeGen(const std::shared_ptr<semant::ClassNode> &root) : _root(root)
{
}

void CodeGen::emit_class_code(const std::shared_ptr<semant::ClassNode> &node)
{
    _current_class = node->_class;
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN CLASS " + _current_class->_type->_string));

    emit_class_code_inner(node);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN CLASS " + _current_class->_type->_string));
}

void CodeGen::emit_binary_expr(const ast::BinaryExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN BINARY EXPR"));

    emit_binary_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN BINARY EXPR"));
}

void CodeGen::emit_unary_expr(const ast::UnaryExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN UNAXRY EXPR"));

    emit_unary_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN UNAXRY EXPR"));
}

void CodeGen::emit_expr(const std::shared_ptr<ast::Expression> &expr)
{
    std::visit(ast::overloaded{[&](const ast::BoolExpression &bool_expr) { emit_bool_expr(bool_expr); },
                               [&](const ast::StringExpression &str) { emit_string_expr(str); },
                               [&](const ast::IntExpression &number) { emit_int_expr(number); },
                               [&](const ast::ObjectExpression &object) { emit_object_expr(object); },
                               [&](const ast::BinaryExpression &binary_expr) { emit_binary_expr(binary_expr); },
                               [&](const ast::UnaryExpression &unary_expr) { emit_unary_expr(unary_expr); },
                               [&](const ast::NewExpression &alloc) { emit_new_expr(alloc); },
                               [&](const ast::CaseExpression &branch) { emit_cases_expr(branch); },
                               [&](const ast::LetExpression &let) { emit_let_expr(let); },
                               [&](const ast::ListExpression &list) { emit_list_expr(list); },
                               [&](const ast::WhileExpression &loop) { emit_loop_expr(loop); },
                               [&](const ast::IfExpression &branch) { emit_if_expr(branch); },
                               [&](const ast::DispatchExpression &dispatch) { emit_dispatch_expr(dispatch); },
                               [&](const ast::AssignExpression &assign) { emit_assign_expr(assign); }},
               expr->_data);
}

void CodeGen::emit_class_init_method(const std::shared_ptr<ast::Class> &klass)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN INIT METHOD FOR CLASS " + _current_class->_type->_string));

    emit_class_init_method_inner(klass);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN INIT METHOD FOR CLASS " + _current_class->_type->_string));
}

void CodeGen::emit_class_method(const std::shared_ptr<ast::Feature> &method)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN METHOD " + method->_object->_object));

    emit_class_method_inner(method);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN METHOD " + method->_object->_object));
}

void CodeGen::emit_object_expr(const ast::ObjectExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN OBJECT EXPR FOR " + expr._object));

    emit_object_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN OBJECT EXPR FOR " + expr._object));
}

void CodeGen::emit_new_expr(const ast::NewExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN NEW EXPR"));

    emit_new_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN NEW EXPR"));
}

void CodeGen::emit_cases_expr(const ast::CaseExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN CASE EXPR"));

    emit_cases_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN CASE EXPR"));
}

void CodeGen::emit_let_expr(const ast::LetExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LET EXPR"));

    emit_let_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN LET EXPR"));
}

void CodeGen::emit_list_expr(const ast::ListExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LIST EXPR"));

    emit_list_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN LIST EXPR"));
}

void CodeGen::emit_loop_expr(const ast::WhileExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LOOP EXPR"));

    emit_loop_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN LOOP EXPR"));
}

void CodeGen::emit_if_expr(const ast::IfExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN IF EXPR"));

    emit_if_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN IF EXPR"));
}

void CodeGen::emit_dispatch_expr(const ast::DispatchExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN DISPATCH EXPR"));

    emit_dispatch_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN DISPATCH EXPR"));
}

void CodeGen::emit_assign_expr(const ast::AssignExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN ASSIGN EXPR"));

    emit_assign_expr_inner(expr);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN ASSIGN EXPR"));
}