#include "ast/AST.h"
#include "utils/Utils.h"
#include <algorithm>
#include <iostream>

namespace ast
{
#ifdef DEBUG
void dump_expression(const int &offset, const std::shared_ptr<Expression> &expr);

void dump_line_and_name(const int &offset, const int &line_number, const std::string &name)
{
    const std::string field(offset, ' ');
    std::cout << field << "#" << line_number << std::endl << field << name << std::endl;
}

void dump_type(const int &offset, const std::shared_ptr<Type> &type)
{
    std::cout << std::string(offset, ' ') << type->_string << std::endl;
}

void dump_object(const int &offset, const std::shared_ptr<ast::ObjectExpression> &object)
{
    std::cout << std::string(offset, ' ') << object->_object << std::endl;
}

template <class T>
concept BinaryExpr = std::is_same_v<T, EqExpression> || std::is_same_v<T, LEExpression> ||
    std::is_same_v<T, LTExpression> || std::is_same_v<T, MinusExpression> || std::is_same_v<T, PlusExpression> ||
    std::is_same_v<T, DivExpression> || std::is_same_v<T, MulExpression>;

template <BinaryExpr T> std::string binary_op_name(const T &expr)
{
    if constexpr (std::is_same_v<T, EqExpression>)
    {
        return "_eq";
    }
    else if (std::is_same_v<T, LEExpression>)
    {
        return "_leq";
    }
    else if (std::is_same_v<T, LTExpression>)
    {
        return "_lt";
    }
    else if (std::is_same_v<T, MinusExpression>)
    {
        return "_sub";
    }
    else if (std::is_same_v<T, PlusExpression>)
    {
        return "_plus";
    }
    else if (std::is_same_v<T, DivExpression>)
    {
        return "_divide";
    }
    else if (std::is_same_v<T, MulExpression>)
    {
        return "_mul";
    }
}

void dump_binary_expr(const BinaryExpression &expr, const int &line, const int &offset)
{
    const std::string field(offset, ' ');
    std::cout << field << "#" << line << std::endl;

    std::visit(overloaded{[&](const BinaryExpr auto &binary_expr) {
                   std::cout << std::string(offset, ' ') << binary_op_name(binary_expr) << std::endl;
               }},
               expr._base);

    dump_expression(offset + 2, expr._lhs);
    dump_expression(offset + 2, expr._rhs);
}

template <class T>
concept UnaryExpr =
    std::is_same_v<T, NegExpression> || std::is_same_v<T, IsVoidExpression> || std::is_same_v<T, NotExpression>;

template <UnaryExpr T> std::string unary_op_name(const T &expr)
{
    if constexpr (std::is_same_v<T, NegExpression>)
    {
        return "_neg";
    }
    else if (std::is_same_v<T, IsVoidExpression>)
    {
        return "_isvoid";
    }
    else if (std::is_same_v<T, NotExpression>)
    {
        return "_comp";
    }
}

void dump_unary_expr(const UnaryExpression &expr, const int &line, const int &offset)
{
    const std::string field(offset, ' ');
    std::cout << field << "#" << line << std::endl;

    std::visit(overloaded{[&](const UnaryExpr auto &unary_expr) {
                   std::cout << std::string(offset, ' ') << unary_op_name(unary_expr) << std::endl;
               }},
               expr._base);

    dump_expression(offset + 2, expr._expr);
}

void dump_case(const int &offset, const std::shared_ptr<Case> &branch)
{
    dump_line_and_name(offset, branch->_line_number, "_branch");

    dump_object(offset + 2, branch->_object);
    dump_type(offset + 2, branch->_type);
    dump_expression(offset + 2, branch->_expr);
}

void dump_dispatch(const int &offset, const DispatchExpression &dispatch)
{
    std::visit(overloaded{[&](const StaticDispatchExpression &static_disp) {
                              std::cout << std::string(offset, ' ') << "_static_dispatch" << std::endl;

                              dump_expression(offset + 2, dispatch._expr);
                              dump_type(offset + 2, static_disp._type);
                          },
                          [&](const VirtualDispatchExpression &obj_disp) {
                              std::cout << std::string(offset, ' ') << "_dispatch" << std::endl;

                              dump_expression(offset + 2, dispatch._expr);
                          }},
               dispatch._base);

    dump_object(offset + 2, dispatch._object);

    const std::string field(offset + 2, ' ');
    std::cout << field << "(" << std::endl;
    std::for_each(dispatch._args.begin(), dispatch._args.end(),
                  [&](const auto &expr) { dump_expression(offset + 2, expr); });
    std::cout << field << ")" << std::endl;
}

void dump_bool_expression(const BoolExpression &bool_expr, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_bool");
    std::cout << std::string(offset + 2, ' ') << (int)bool_expr._value << std::endl;
}

void dump_string_expression(const StringExpression &str, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_string");
    std::cout << std::string(offset + 2, ' ') << printable_string(str._string) << std::endl;
}

void dump_int_expression(const IntExpression &number, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_int");
    std::cout << std::string(offset + 2, ' ') << number._value << std::endl;
}

void dump_object_expression(const ObjectExpression &object, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_object");
    std::cout << std::string(offset + 2, ' ') << object._object << std::endl;
}

void dump_new_expression(const NewExpression &alloc, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_new");
    dump_type(offset + 2, alloc._type);
}

void dump_case_expression(const CaseExpression &branch, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_typcase");
    dump_expression(offset + 2, branch._expr);
    std::for_each(branch._cases.begin(), branch._cases.end(),
                  [&offset](const auto &branch) { dump_case(offset + 2, branch); });
}

void dump_let_expression(const LetExpression &let, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_let");
    dump_object(offset + 2, let._object);
    dump_type(offset + 2, let._type);
    dump_expression(offset + 2, let._expr);
    dump_expression(offset + 2, let._body_expr);
}

void dump_list_expression(const ListExpression &list, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_block");
    std::for_each(list._exprs.begin(), list._exprs.end(), [&offset](const auto &e) { dump_expression(offset + 2, e); });
}

void dump_while_expression(const WhileExpression &loop, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_loop");
    dump_expression(offset + 2, loop._predicate);
    dump_expression(offset + 2, loop._body_expr);
}

void dump_if_expression(const IfExpression &branch, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_cond");
    dump_expression(offset + 2, branch._predicate);
    dump_expression(offset + 2, branch._true_path_expr);
    dump_expression(offset + 2, branch._false_path_expr);
}

void dump_dispatch_expression(const DispatchExpression &dispatch, const int &line, const int &offset)
{
    const std::string field(offset, ' ');
    std::cout << field << "#" << line << std::endl;
    dump_dispatch(offset, dispatch);
}

void dump_assign_expression(const AssignExpression &assign, const int &line, const int &offset)
{
    dump_line_and_name(offset, line, "_assign");
    dump_object(offset + 2, assign._object);
    dump_expression(offset + 2, assign._expr);
}

void dump_expression(const int &offset, const std::shared_ptr<Expression> &expr)
{
    if (expr == nullptr)
    {
        dump_line_and_name(offset, 0, "_no_expr");
        std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;

        return;
    }

    std::visit(
        overloaded{
            [&](const BoolExpression &bool_expr) { dump_bool_expression(bool_expr, expr->_line_number, offset); },
            [&](const StringExpression &str) { dump_string_expression(str, expr->_line_number, offset); },
            [&](const IntExpression &number) { dump_int_expression(number, expr->_line_number, offset); },
            [&](const ObjectExpression &object) { dump_object_expression(object, expr->_line_number, offset); },
            [&](const BinaryExpression &binary_expr) { dump_binary_expr(binary_expr, expr->_line_number, offset); },
            [&](const UnaryExpression &unary_expr) { dump_unary_expr(unary_expr, expr->_line_number, offset); },
            [&](const NewExpression &alloc) { dump_new_expression(alloc, expr->_line_number, offset); },
            [&](const CaseExpression &branch) { dump_case_expression(branch, expr->_line_number, offset); },
            [&](const LetExpression &let) { dump_let_expression(let, expr->_line_number, offset); },
            [&](const ListExpression &list) { dump_list_expression(list, expr->_line_number, offset); },
            [&](const WhileExpression &loop) { dump_while_expression(loop, expr->_line_number, offset); },
            [&](const IfExpression &branch) { dump_if_expression(branch, expr->_line_number, offset); },
            [&](const DispatchExpression &dispatch) { dump_dispatch_expression(dispatch, expr->_line_number, offset); },
            [&](const AssignExpression &assign) { dump_assign_expression(assign, expr->_line_number, offset); }},
        expr->_data);

    std::cout << std::string(offset, ' ') << ": " << (expr->_type ? expr->_type->_string : "_no_type") << std::endl;
}

void dump_formal(const int &offset, const std::shared_ptr<Formal> &formal)
{
    dump_line_and_name(offset, formal->_line_number, "_formal");

    dump_object(offset + 2, formal->_object);
    dump_type(offset + 2, formal->_type);
}

void dump_feature(const int &offset, const std::shared_ptr<Feature> &feature)
{
    const std::string field(offset, ' ');
    std::cout << field << "#" << feature->_line_number << std::endl;

    std::visit(overloaded{[&](const AttrFeature &attr) {
                              std::cout << field << "_attr" << std::endl;
                              dump_object(offset + 2, feature->_object);
                          },
                          [&](const MethodFeature &method) {
                              std::cout << field << "_method" << std::endl;
                              dump_object(offset + 2, feature->_object);

                              std::for_each(method._formals.begin(), method._formals.end(),
                                            [&offset](const auto &formal) { dump_formal(offset + 2, formal); });
                          }},
               feature->_base);

    dump_type(offset + 2, feature->_type);
    dump_expression(offset + 2, feature->_expr);
}

void dump_class(const int &offset, const std::shared_ptr<Class> &klass)
{
    dump_line_and_name(offset, klass->_line_number, "_class");

    dump_type(offset + 2, klass->_type);
    dump_type(offset + 2, klass->_parent);
    const std::string field(offset + 2, ' ');
    std::cout << field << "\"" << klass->_file_name << "\"" << std::endl << field << "(" << std::endl;

    std::for_each(klass->_features.begin(), klass->_features.end(),
                  [&offset](const auto &feature) { dump_feature(offset + 2, feature); });

    std::cout << field << ")" << std::endl;
}

void dump_program(const Program &program)
{

    dump_line_and_name(0, program._line_number, "_program");

    std::for_each(program._classes.begin(), program._classes.end(), [](const auto &klass) { dump_class(2, klass); });
}
#endif // DEBUG
} // namespace ast
