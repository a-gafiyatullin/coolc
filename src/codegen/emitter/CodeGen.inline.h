#pragma once

#include "codegen/emitter/CodeGen.h"
#include "utils/logger/Logger.h"

using namespace codegen;

#define CODEGEN_RETURN_VALUE_IF_CAN(expression, message)                                                               \
    if constexpr (std::is_same_v<Value, void>)                                                                         \
    {                                                                                                                  \
        expression;                                                                                                    \
        CODEGEN_VERBOSE_ONLY(LOG_EXIT(message));                                                                       \
                                                                                                                       \
        return;                                                                                                        \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        const Value res = expression;                                                                                  \
        CODEGEN_VERBOSE_ONLY(LOG_EXIT(message));                                                                       \
                                                                                                                       \
        return res;                                                                                                    \
    }

template <class Value, class Symbol>
CodeGen<Value, Symbol>::CodeGen(const std::shared_ptr<KlassBuilder> &builder) : _builder(builder)
{
}

template <class Value, class Symbol>
void CodeGen<Value, Symbol>::emit_class_code(const std::shared_ptr<semant::ClassNode> &node)
{
    _current_class = node->_class;
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN CLASS \"" + _current_class->_type->_string + "\""));

    // create new scope and add all fields
    _table.push_scope();
    add_fields();

    // emit methods
    emit_class_init_method();
    for (const auto &feature : _current_class->_features)
    {
        if (std::holds_alternative<ast::MethodFeature>(feature->_base))
        {
            emit_class_method(feature);
        }
    }

    // process children
    for (const auto &node : node->_children)
    {
        emit_class_code(node);
    }

    // delete scope
    _table.pop_scope();

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN CLASS \"" + _current_class->_type->_string + "\""));
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_binary_expr(const ast::BinaryExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN BINARY EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_binary_expr_inner(expr, expr_type), "GEN BINARY EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_unary_expr(const ast::UnaryExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN UNARY EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_unary_expr_inner(expr, expr_type), "GEN UNARY EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_expr(const std::shared_ptr<ast::Expression> &expr)
{
    return std::visit(
        ast::overloaded{
            [&](const ast::BoolExpression &bool_expr) { return emit_bool_expr(bool_expr, expr->_type); },
            [&](const ast::StringExpression &str) { return emit_string_expr(str, expr->_type); },
            [&](const ast::IntExpression &number) { return emit_int_expr(number, expr->_type); },
            [&](const ast::ObjectExpression &object) { return emit_object_expr(object, expr->_type); },
            [&](const ast::BinaryExpression &binary_expr) { return emit_binary_expr(binary_expr, expr->_type); },
            [&](const ast::UnaryExpression &unary_expr) { return emit_unary_expr(unary_expr, expr->_type); },
            [&](const ast::NewExpression &alloc) { return emit_new_expr(alloc, expr->_type); },
            [&](const ast::CaseExpression &branch) { return emit_cases_expr(branch, expr->_type); },
            [&](const ast::LetExpression &let) { return emit_let_expr(let, expr->_type); },
            [&](const ast::ListExpression &list) { return emit_list_expr(list, expr->_type); },
            [&](const ast::WhileExpression &loop) { return emit_loop_expr(loop, expr->_type); },
            [&](const ast::IfExpression &branch) { return emit_if_expr(branch, expr->_type); },
            [&](const ast::DispatchExpression &dispatch) { return emit_dispatch_expr(dispatch, expr->_type); },
            [&](const ast::AssignExpression &assign) { return emit_assign_expr(assign, expr->_type); }},
        expr->_data);
}

template <class Value, class Symbol> void CodeGen<Value, Symbol>::emit_class_init_method()
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN INIT METHOD FOR CLASS \"" + _current_class->_type->_string + "\""));

    emit_class_init_method_inner();

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN INIT METHOD FOR CLASS \"" + _current_class->_type->_string + "\""));
}

template <class Value, class Symbol>
void CodeGen<Value, Symbol>::emit_class_method(const std::shared_ptr<ast::Feature> &method)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN METHOD \"" + method->_object->_object + "\""));

    emit_class_method_inner(method);

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN METHOD \"" + method->_object->_object + "\""));
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_object_expr(const ast::ObjectExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN OBJECT EXPR FOR \"" + expr._object + "\""));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_object_expr_inner(expr, expr_type), "GEN OBJECT EXPR FOR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_new_expr(const ast::NewExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN NEW EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_new_expr_inner(expr, expr_type), "GEN NEW EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_cases_expr(const ast::CaseExpression &expr,
                                              const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN CASE EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_cases_expr_inner(expr, expr_type), "GEN CASE EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_let_expr(const ast::LetExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LET EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_let_expr_inner(expr, expr_type), "GEN LET EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_list_expr(const ast::ListExpression &expr,
                                             const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LIST EXPR"));

    if constexpr (std::is_same_v<Value, void>)
    {
        for (const auto &e : expr._exprs)
        {
            emit_expr(e);
        }
    }
    else
    {
        Value res;
        for (const auto &e : expr._exprs)
        {
            res = emit_expr(e);
        }

        CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN LIST EXPR"));
        return res;
    }

    CODEGEN_VERBOSE_ONLY(LOG_EXIT("GEN LIST EXPR"));
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_loop_expr(const ast::WhileExpression &expr,
                                             const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LOOP EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_loop_expr_inner(expr, expr_type), "GEN LOOP EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_if_expr(const ast::IfExpression &expr, const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN IF EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_if_expr_inner(expr, expr_type), "GEN IF EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_dispatch_expr(const ast::DispatchExpression &expr,
                                                 const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN DISPATCH EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_dispatch_expr_inner(expr, expr_type), "GEN DISPATCH EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_assign_expr(const ast::AssignExpression &expr,
                                               const std::shared_ptr<ast::Type> &expr_type)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN ASSIGN EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_assign_expr_inner(expr, expr_type), "GEN ASSIGN EXPR");
}
