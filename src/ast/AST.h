#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#ifdef DEBUG
#include <algorithm>
#include <iostream>
#endif // DEBUG

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

    int _line_number;
};

struct Class
{
    std::shared_ptr<Type> _type;
    std::shared_ptr<Type> _parent;
    std::vector<std::shared_ptr<Feature>> _features;

    std::string _file_name;
    int _line_number;
    int _expression_stack; // minimal number of slots for expression stack of this init for his class
};

struct AttrFeature
{
};

struct MethodFeature
{
    std::vector<std::shared_ptr<Formal>> _formals;
    int _expression_stack; // minimal number of slots for expression stack of this method
};

struct Feature
{
    std::variant<AttrFeature, MethodFeature> _base;

    std::shared_ptr<ObjectExpression> _object;
    std::shared_ptr<Type> _type;
    std::shared_ptr<Expression> _expr;

    int _line_number;
};

struct Formal
{
    std::shared_ptr<ObjectExpression> _object;
    std::shared_ptr<Type> _type;

    int _line_number;
};

struct Case
{
    std::shared_ptr<ObjectExpression> _object;
    std::shared_ptr<Type> _type;
    std::shared_ptr<Expression> _expr;

    int _line_number;
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

struct VirtualDispatchExpression
{
};

struct DispatchExpression
{
    std::variant<VirtualDispatchExpression, StaticDispatchExpression> _base;

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
    std::variant<EqExpression, LEExpression, LTExpression, MinusExpression, PlusExpression, DivExpression,
                 MulExpression>
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
    std::variant<AssignExpression, DispatchExpression, BinaryExpression, UnaryExpression, IfExpression, WhileExpression,
                 ListExpression, LetExpression, CaseExpression, NewExpression, ObjectExpression, IntExpression,
                 StringExpression, BoolExpression>
        _data;

    int _line_number;

    std::shared_ptr<Type> _type;
    bool _can_allocate = false;
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#ifdef DEBUG
/**
 * @brief Print program AST
 *
 * @param program Program instance
 */
void dump_program(const Program &program);
#endif // DEBUG
} // namespace ast
