#include "ast/AST.h"

namespace ast
{
#ifdef PARSER_VERBOSE
    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    void dump_expression(const int &offset, const std::shared_ptr<Expression> &expr);

    void dump_line_and_name(const int &offset, const int &line_number, const std::string &name)
    {
        std::string field(offset, ' ');
        std::cout << field << "#" << line_number << std::endl
                  << field << name << std::endl;
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
    concept BinaryExpr = std::is_same_v<T, EqExpression> ||
        std::is_same_v<T, LEExpression> || std::is_same_v<T, LTExpression> ||
        std::is_same_v<T, MinusExpression> || std::is_same_v<T, PlusExpression> ||
        std::is_same_v<T, DivExpression> || std::is_same_v<T, MulExpression>;

    template <BinaryExpr T>
    std::string get_binary_op_name(const T &expr)
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
        else if constexpr (std::is_same_v<T, MinusExpression>)
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

    void dump_binary_expr(const int &offset, const BinaryExpression &expr)
    {
        std::visit(overloaded{[&](const BinaryExpr auto &binary_expr)
                              {
                                  std::cout << std::string(offset, ' ')
                                            << get_binary_op_name(binary_expr) << std::endl;
                              }},
                   expr._base);

        dump_expression(offset + 2, expr._lhs);
        dump_expression(offset + 2, expr._rhs);
    }

    template <class T>
    concept UnaryExpr = std::is_same_v<T, NegExpression> ||
        std::is_same_v<T, IsVoidExpression> || std::is_same_v<T, NotExpression>;

    template <UnaryExpr T>
    std::string get_unary_op_name(const T &expr)
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

    void dump_unary_expr(const int &offset, const UnaryExpression &expr)
    {
        std::visit(overloaded{[&](const UnaryExpr auto &unary_expr)
                              {
                                  std::cout << std::string(offset, ' ')
                                            << get_unary_op_name(unary_expr) << std::endl;
                              }},
                   expr._base);

        dump_expression(offset + 2, expr._expr);
    }

    void dump_case(const int &offset, const std::shared_ptr<Case> &case_)
    {
        dump_line_and_name(offset, case_->_line_number, "_branch");

        dump_object(offset + 2, case_->_object);
        dump_type(offset + 2, case_->_type);
        dump_expression(offset + 2, case_->_expr);
    }

    void dump_dispatch(const int &offset, const DispatchExpression &dispatch)
    {
        std::visit(overloaded{[&](const StaticDispatchExpression &static_disp)
                              {
                                  std::cout << std::string(offset, ' ') << "_static_dispatch" << std::endl;

                                  dump_expression(offset + 2, dispatch._expr);
                                  dump_type(offset + 2, static_disp._type);
                              },
                              [&](const ObjectDispatchExpression &obj_disp)
                              {
                                  std::cout << std::string(offset, ' ') << "_dispatch" << std::endl;

                                  dump_expression(offset + 2, dispatch._expr);
                              }},
                   dispatch._base);

        dump_object(offset + 2, dispatch._object);

        std::string field(offset + 2, ' ');
        std::cout << field << "(" << std::endl;
        std::for_each(dispatch._args.begin(), dispatch._args.end(), [&](const auto &expr)
                      { dump_expression(offset + 2, expr); });
        std::cout << field << ")" << std::endl;
    }

    void dump_expression(const int &offset, const std::shared_ptr<Expression> &expr)
    {
        if (expr == nullptr)
        {
            dump_line_and_name(offset, 0, "_no_expr");
            std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;

            return;
        }

        std::visit(overloaded{[&](const BoolExpression &bool_expr)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_bool");

                                  std::cout << std::string(offset + 2, ' ') << (int)bool_expr._value << std::endl;
                              },
                              [&](const StringExpression &str)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_string");

                                  std::cout << std::string(offset + 2, ' ') << get_printable_string(str._string) << std::endl;
                              },
                              [&](const IntExpression &number)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_int");

                                  std::cout << std::string(offset + 2, ' ') << number._value << std::endl;
                              },
                              [&](const ObjectExpression &object)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_object");

                                  std::cout << std::string(offset + 2, ' ') << object._object << std::endl;
                              },
                              [&](const BinaryExpression &binary_expr)
                              {
                                  std::string field(offset, ' ');
                                  std::cout << field << "#" << expr->_line_number << std::endl;

                                  dump_binary_expr(offset, binary_expr);
                              },
                              [&](const UnaryExpression &unary_expr)
                              {
                                  std::string field(offset, ' ');
                                  std::cout << field << "#" << expr->_line_number << std::endl;

                                  dump_unary_expr(offset, unary_expr);
                              },
                              [&](const NewExpression &new_)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_new");

                                  dump_type(offset + 2, new_._type);
                              },
                              [&](const CaseExpression &case_)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_typcase");

                                  dump_expression(offset + 2, case_._expr);
                                  std::for_each(case_._cases.begin(), case_._cases.end(),
                                                [&offset](const auto &branch)
                                                { dump_case(offset + 2, branch); });
                              },
                              [&](const LetExpression &let)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_let");

                                  dump_object(offset + 2, let._object);
                                  dump_type(offset + 2, let._type);
                                  dump_expression(offset + 2, let._expr);
                                  dump_expression(offset + 2, let._body_expr);
                              },
                              [&](const ListExpression &list)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_block");

                                  std::for_each(list._exprs.begin(), list._exprs.end(),
                                                [&offset](const auto &e)
                                                { dump_expression(offset + 2, e); });
                              },
                              [&](const WhileExpression &while_)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_loop");

                                  dump_expression(offset + 2, while_._predicate);
                                  dump_expression(offset + 2, while_._body_expr);
                              },
                              [&](const IfExpression &if_)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_cond");

                                  dump_expression(offset + 2, if_._predicate);
                                  dump_expression(offset + 2, if_._true_path_expr);
                                  dump_expression(offset + 2, if_._false_path_expr);
                              },
                              [&](const DispatchExpression &dispatch)
                              {
                                  std::string field(offset, ' ');
                                  std::cout << field << "#" << expr->_line_number << std::endl;

                                  dump_dispatch(offset, dispatch);
                              },
                              [&](const AssignExpression &assign)
                              {
                                  dump_line_and_name(offset, expr->_line_number, "_assign");

                                  dump_object(offset + 2, assign._object);
                                  dump_expression(offset + 2, assign._expr);
                              }},
                   expr->_data);

        std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;
    }

    void dump_formal(const int &offset, const std::shared_ptr<Formal> &formal)
    {
        dump_line_and_name(offset, formal->_line_number, "_formal");

        dump_object(offset + 2, formal->_object);
        dump_type(offset + 2, formal->_type);
    }

    void dump_feature(const int &offset, const std::shared_ptr<Feature> &feature)
    {
        std::string field(offset, ' ');
        std::cout << field << "#" << feature->_line_number << std::endl;

        std::visit(overloaded{[&](const AttrFeature &attr)
                              {
                                  std::cout << field << "_attr" << std::endl;
                                  dump_object(offset + 2, feature->_object);
                              },
                              [&](const MethodFeature &method)
                              {
                                  std::cout << field << "_method" << std::endl;
                                  dump_object(offset + 2, feature->_object);

                                  std::for_each(method._formals.begin(), method._formals.end(),
                                                [&offset](const auto &formal)
                                                { dump_formal(offset + 2, formal); });
                              }},
                   feature->_base);

        dump_type(offset + 2, feature->_type);
        dump_expression(offset + 2, feature->_expr);
    }

    void dump_class(const int &offset, const std::shared_ptr<Class> &class_)
    {
        dump_line_and_name(offset, class_->_line_number, "_class");

        dump_type(offset + 2, class_->_type);
        dump_type(offset + 2, class_->_parent);
        std::string field(offset + 2, ' ');
        std::cout << field << "\"" << class_->_file_name << "\"" << std::endl
                  << field << "(" << std::endl;

        std::for_each(class_->_features.begin(), class_->_features.end(),
                      [&offset](const auto &feature)
                      { dump_feature(offset + 2, feature); });

        std::cout << field << ")" << std::endl;
    }

    void dump_program(const std::shared_ptr<Program> &program)
    {

        dump_line_and_name(0, program->_line_number, "_program");

        std::for_each(program->_classes.begin(), program->_classes.end(),
                      [](const auto &class_)
                      { dump_class(2, class_); });
    }
#endif // PARSER_VERBOSE
}