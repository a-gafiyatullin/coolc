#pragma once

#include "decls/Decls.h"
#include "semant/scope/Scope.h"
#include <algorithm>

#define SEMANT_RETURN_IF_FALSE_WITH_ERROR(cond, error, line_num, retval)                                               \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        _error_message = error;                                                                                        \
        _error_line_number = line_num;                                                                                 \
        return retval;                                                                                                 \
    }

namespace semant
{
struct ClassNode
{
    std::shared_ptr<ast::Class> _class;
    std::vector<std::shared_ptr<ClassNode>> _children;
};

class Semant
{
  private:
    static constexpr std::string_view EMPTY_TYPE_NAME = "_EMPTY_TYPE";

    // create error report from message, file name and line number
    std::string _error_message;
    std::string _error_file_name;
    int _error_line_number;

    // ----------------------------- Initialization -----------------------------
    // merge multiple programs were generated by parser to one program
    static std::shared_ptr<ast::Program> merge_to_one_program(
        const std::vector<std::shared_ptr<ast::Program>> &programs);
    std::shared_ptr<ast::Program> _program;    // program to analyze
    std::shared_ptr<ast::Type> _current_class; // current class of analysis for SELF_TYPE

    // ----------------------------- Analysis algorithms support -----------------------------
    std::unordered_map<std::string, std::shared_ptr<ClassNode>> _classes; // fast access to class info
    std::shared_ptr<ClassNode> _root;                                     // root of classes

    // ----------------------------- Class checking -----------------------------
    // creates dummy class with methods:
    // methods array: [ ("method1", ["ret_type1", "type1", "type2"]),
    //                  ("method2", ["ret_type2", "type3", "type4"]) ]
    std::shared_ptr<ClassNode> create_basic_class(
        const std::string &name, const std::string &parent,
        const std::vector<std::pair<std::string, std::vector<std::string>>> &methods,
        const std::vector<std::shared_ptr<ast::Type>> &fields);

    // class check
    bool check_classes();
    bool check_class_hierarchy();
    bool check_main();
    // class check helpers
    bool check_class_hierarchy_for_cycle(const std::shared_ptr<ClassNode> &klass,
                                         std::unordered_map<std::string, int> &visited, const int &loop);
    static bool is_inherit_allowed(const std::shared_ptr<ast::Type> &klass);

    // ----------------------------- Expression checking -----------------------------
    // expressions type check
    bool check_expressions();
    bool check_expressions_in_class(const std::shared_ptr<ClassNode> &node, Scope &scope);
    bool check_expression_in_method(const std::shared_ptr<ast::Feature> &method, Scope &scope);
    bool check_expression_in_attribute(const std::shared_ptr<ast::Feature> &attr, Scope &scope);
    bool infer_expression_type(const std::shared_ptr<ast::Expression> &expr, Scope &scope);

    std::shared_ptr<ast::Type> infer_new_type(const ast::NewExpression &alloc);
    std::shared_ptr<ast::Type> infer_let_type(const ast::LetExpression &let, Scope &scope);
    std::shared_ptr<ast::Type> infer_loop_type(const ast::WhileExpression &loop, Scope &scope);
    std::shared_ptr<ast::Type> infer_unary_type(const ast::UnaryExpression &unary, Scope &scope);
    std::shared_ptr<ast::Type> infer_binary_type(const ast::BinaryExpression &binary, Scope &scope);
    std::shared_ptr<ast::Type> infer_assign_type(const ast::AssignExpression &assign, Scope &scope);
    std::shared_ptr<ast::Type> infer_if_type(const ast::IfExpression &branch, Scope &scope);
    std::shared_ptr<ast::Type> infer_sequence_type(const ast::ListExpression &seq, Scope &scope);
    std::shared_ptr<ast::Type> infer_cases_type(const ast::CaseExpression &cases, Scope &scope);
    std::shared_ptr<ast::Type> infer_dispatch_type(const ast::DispatchExpression &disp, Scope &scope);
    std::shared_ptr<ast::Type> infer_object_type(const ast::ObjectExpression &obj, Scope &scope);

    // ----------------------------- Type checking support -----------------------------
    // basic types
    static std::shared_ptr<ast::Type> Bool;
    static std::shared_ptr<ast::Type> Object;
    static std::shared_ptr<ast::Type> Int;
    static std::shared_ptr<ast::Type> String;
    static std::shared_ptr<ast::Type> Io;
    static std::shared_ptr<ast::Type> SelfType;
    static std::shared_ptr<ast::Type> EmptyType; // special type for no type

    // expressions type check helpers

    // t1 is subtype of t2
    // this function is not сommutative !!!
    bool check_types_meet(const std::shared_ptr<ast::Type> &dynamic_type,
                          const std::shared_ptr<ast::Type> &static_type) const;
    inline static bool same_type(const std::shared_ptr<ast::Type> &t1, const std::shared_ptr<ast::Type> &t2)
    {
        return t1->_string == t2->_string;
    }

    std::shared_ptr<ast::Type> exact_type(const std::shared_ptr<ast::Type> &type) const;
    std::shared_ptr<ast::Type> find_common_ancestor(const std::vector<std::shared_ptr<ast::Type>> &classes) const;
    std::shared_ptr<ast::Type> find_common_ancestor_of_two(const std::shared_ptr<ast::Type> &t1,
                                                           const std::shared_ptr<ast::Type> &t2) const;
    std::shared_ptr<ast::Feature> find_method(const std::string &name, const std::shared_ptr<ast::Type> &klass,
                                              const bool &exact) const;
    inline bool check_exists(const std::shared_ptr<ast::Type> &type) const
    {
        return _classes.find(type->_string) != _classes.end() || is_empty_type(type);
    }

  public:
    /**
     * @brief Construct a new Semant
     *
     * @param vector Vector of programs that were generated by parser
     */
    explicit Semant(std::vector<std::shared_ptr<ast::Program>> programs);

    /**
     * @brief Infer empty types in AST
     *
     * @return Root of class hierarhy program that consists of classes from all programs
     */
    std::pair<std::shared_ptr<ClassNode>, std::shared_ptr<ast::Program>> infer_types_and_check();

    /**
     * @brief Check if type is basic type
     *
     * @param type Type for check
     * @return True if type is basic class
     */
    static bool is_basic_type(const std::shared_ptr<ast::Type> &type);

    /**
     * @brief Check if type is trivial
     *
     * @param type Type for check
     * @return True if type is trivial
     */
    static bool is_trivial_type(const std::shared_ptr<ast::Type> &type);

    /**
     * @brief Check if type is boolean
     *
     * @param t Type for check
     * @return True if type is boolean
     */
    inline static bool is_bool(const std::shared_ptr<ast::Type> &t)
    {
        return same_type(t, Bool);
    }

    /**
     * @brief Check if type is int
     *
     * @param t Type for check
     * @return True if type is int
     */
    inline static bool is_int(const std::shared_ptr<ast::Type> &t)
    {
        return same_type(t, Int);
    }

    /**
     * @brief Check if type is string
     *
     * @param t Type for check
     * @return True if type is string
     */
    inline static bool is_string(const std::shared_ptr<ast::Type> &t)
    {
        return same_type(t, String);
    }

    /**
     * @brief Check if type is SELF_TYPE
     *
     * @param t Type for check
     * @return True if type is SELF_TYPE
     */
    inline static bool is_self_type(const std::shared_ptr<ast::Type> &t)
    {
        return same_type(t, SelfType);
    }

    /**
     * @brief Check if type is _EMPTY_TYPE
     *
     * @param t Type for check
     * @return True if type is _EMPTY_TYPE
     */
    inline static bool is_empty_type(const std::shared_ptr<ast::Type> &t)
    {
        return same_type(t, EmptyType);
    }

    /**
     * @brief Get EmptyType type pointer
     *
     * @return EmptyType type pointer
     */
    inline static std::shared_ptr<ast::Type> empty_type()
    {
        return EmptyType;
    }

    /**
     * @brief Calculate exact type for a given type
     *
     * @param ltype Type for check
     * @param rtype Current type
     * @return rtype if type is SELF_TYPE or ltype
     */
    static std::shared_ptr<ast::Type> exact_type(const std::shared_ptr<ast::Type> &ltype,
                                                 const std::shared_ptr<ast::Type> &rtype);

    /**
     * @brief Get the error message
     *
     * @return Error message
     */
    std::string error_msg() const;
};
} // namespace semant