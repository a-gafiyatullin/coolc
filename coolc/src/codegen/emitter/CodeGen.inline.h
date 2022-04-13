#include "codegen/emitter/CodeGen.h"

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

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_binary_expr(const ast::BinaryExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN BINARY EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_binary_expr_inner(expr), "GEN BINARY EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_unary_expr(const ast::UnaryExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN UNAXRY EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_unary_expr_inner(expr), "GEN UNAXRY EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_expr(const std::shared_ptr<ast::Expression> &expr)
{
    return std::visit(
        ast::overloaded{
            // TODO: do we really need specify ret type?
            [&](const ast::BoolExpression &bool_expr) -> Value { return emit_bool_expr(bool_expr); },
            [&](const ast::StringExpression &str) -> Value { return emit_string_expr(str); },
            [&](const ast::IntExpression &number) -> Value { return emit_int_expr(number); },
            [&](const ast::ObjectExpression &object) -> Value { return emit_object_expr(object); },
            [&](const ast::BinaryExpression &binary_expr) -> Value { return emit_binary_expr(binary_expr); },
            [&](const ast::UnaryExpression &unary_expr) -> Value { return emit_unary_expr(unary_expr); },
            [&](const ast::NewExpression &alloc) -> Value { return emit_new_expr(alloc); },
            [&](const ast::CaseExpression &branch) -> Value { return emit_cases_expr(branch); },
            [&](const ast::LetExpression &let) -> Value { return emit_let_expr(let); },
            [&](const ast::ListExpression &list) -> Value { return emit_list_expr(list); },
            [&](const ast::WhileExpression &loop) -> Value { return emit_loop_expr(loop); },
            [&](const ast::IfExpression &branch) -> Value { return emit_if_expr(branch); },
            [&](const ast::DispatchExpression &dispatch) -> Value { return emit_dispatch_expr(dispatch); },
            [&](const ast::AssignExpression &assign) -> Value { return emit_assign_expr(assign); }},
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

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_object_expr(const ast::ObjectExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN OBJECT EXPR FOR \"" + expr._object + "\""));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_object_expr_inner(expr), "GEN OBJECT EXPR FOR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_new_expr(const ast::NewExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN NEW EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_new_expr_inner(expr), "GEN NEW EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_cases_expr(const ast::CaseExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN CASE EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_cases_expr_inner(expr), "GEN CASE EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_let_expr(const ast::LetExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LET EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_let_expr_inner(expr), "GEN LET EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_list_expr(const ast::ListExpression &expr)
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

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_loop_expr(const ast::WhileExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN LOOP EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_loop_expr_inner(expr), "GEN LOOP EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_if_expr(const ast::IfExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN IF EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_if_expr_inner(expr), "GEN IF EXPR");
}

template <class Value, class Symbol>
Value CodeGen<Value, Symbol>::emit_dispatch_expr(const ast::DispatchExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN DISPATCH EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_dispatch_expr_inner(expr), "GEN DISPATCH EXPR");
}

template <class Value, class Symbol> Value CodeGen<Value, Symbol>::emit_assign_expr(const ast::AssignExpression &expr)
{
    CODEGEN_VERBOSE_ONLY(LOG_ENTER("GEN ASSIGN EXPR"));

    CODEGEN_RETURN_VALUE_IF_CAN(emit_assign_expr_inner(expr), "GEN ASSIGN EXPR");
}