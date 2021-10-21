#pragma once

#include <vector>
#include <variant>
#include <memory>
#include "utils/Utils.h"

#ifdef PARSER_VERBOSE
#include <iostream>
#include <algorithm>
#endif // PARSER_VERBOSE

namespace ast
{
    // main structural elements
    struct Program;
    struct Class;
    struct Feature;
    struct Formal;
    struct Case;
    struct Expression;

    struct Type;
    struct ObjectExpression;

    struct Program
    {
        std::vector<std::shared_ptr<Class>> _classes;

        PARSER_VERBOSE_ONLY(int _line_number;);
    };

    struct Class
    {
        std::shared_ptr<Type> _type;
        std::shared_ptr<Type> _parent;
        std::vector<std::shared_ptr<Feature>> _features;

        PARSER_VERBOSE_ONLY(std::string _file_name;);
        PARSER_VERBOSE_ONLY(int _line_number;);
    };

    struct AttrFeature
    {
    };

    struct MethodFeature
    {
        std::vector<std::shared_ptr<Formal>> _formals;
    };

    struct Feature
    {
        std::variant<AttrFeature, MethodFeature> _base;

        std::shared_ptr<ObjectExpression> _object;
        std::shared_ptr<Type> _type;
        std::shared_ptr<Expression> _expr;

        PARSER_VERBOSE_ONLY(int _line_number;);
    };

    struct Formal
    {
        std::shared_ptr<ObjectExpression> _object;
        std::shared_ptr<Type> _type;

        PARSER_VERBOSE_ONLY(int _line_number;);
    };

    struct Case
    {
        std::shared_ptr<ObjectExpression> _object;
        std::shared_ptr<Type> _type;
        std::shared_ptr<Expression> _expr;

        PARSER_VERBOSE_ONLY(int _line_number;);
    };

    struct AssignExpression
    {
        std::shared_ptr<ObjectExpression> _object;
        std::shared_ptr<Expression> _expr;
    };

    struct StaticDispatchExpression
    {
        std::shared_ptr<Type> _type;
    };

    struct ObjectDispatchExpression
    {
    };

    struct DispatchExpression
    {
        std::variant<ObjectDispatchExpression, StaticDispatchExpression> _base;

        std::shared_ptr<Expression> _expr;
        std::shared_ptr<ObjectExpression> _object;
        std::vector<std::shared_ptr<Expression>> _args;
    };

    struct IfExpression
    {
        std::shared_ptr<Expression> _predicate;
        std::shared_ptr<Expression> _true_path_expr;
        std::shared_ptr<Expression> _false_path_expr;
    };

    struct WhileExpression
    {
        std::shared_ptr<Expression> _predicate;
        std::shared_ptr<Expression> _body_expr;
    };

    struct ListExpression
    {
        std::vector<std::shared_ptr<Expression>> _exprs;
    };

    struct LetExpression
    {
        std::shared_ptr<ObjectExpression> _object;
        std::shared_ptr<Type> _type;
        std::shared_ptr<Expression> _expr;

        std::shared_ptr<Expression> _body_expr;
    };

    struct CaseExpression
    {
        std::shared_ptr<Expression> _expr;
        std::vector<std::shared_ptr<Case>> _cases;
    };

    struct NewExpression
    {
        std::shared_ptr<Type> _type;
    };

    // unary expressions
    struct NotExpression
    {
    };

    struct IsVoidExpression
    {
    };

    struct NegExpression
    {
    };

    struct UnaryExpression
    {
        std::variant<NegExpression, IsVoidExpression, NotExpression> _base;
        std::shared_ptr<Expression> _expr;
    };

    // binary expressions
    struct MulExpression
    {
    };

    struct DivExpression
    {
    };

    struct PlusExpression
    {
    };

    struct MinusExpression
    {
    };

    struct LTExpression
    {
    };

    struct LEExpression
    {
    };

    struct EqExpression
    {
    };

    struct BinaryExpression
    {
        std::variant<EqExpression, LEExpression, LTExpression,
                     MinusExpression, PlusExpression, DivExpression, MulExpression>
            _base;

        std::shared_ptr<Expression> _lhs;
        std::shared_ptr<Expression> _rhs;
    };

    // Atoms
    struct ObjectExpression
    {
        std::string _object;
    };

    struct IntExpression
    {
        int _value;
    };

    struct StringExpression
    {
        std::string _string;
    };

    struct BoolExpression
    {
        bool _value;
    };

    struct Type
    {
        std::string _string;
    };

    struct Expression
    {
        std::variant<AssignExpression, DispatchExpression, BinaryExpression, UnaryExpression,
                     IfExpression, WhileExpression, ListExpression, LetExpression, CaseExpression,
                     NewExpression, ObjectExpression, IntExpression, StringExpression, BoolExpression>
            _data;

        PARSER_VERBOSE_ONLY(int _line_number);
    };

#ifdef PARSER_VERBOSE
    // print ast
    void dump_program(const Program &program);
#endif // PARSER_VERBOSE
}
