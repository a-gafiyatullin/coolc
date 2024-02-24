#pragma once

#include "decls/Decls.h"
#include "lexer/Lexer.h"
#include <functional>
#include <stack>

#ifdef DEBUG
#define PARSER_APPEND_LINE_NUM(msg)                                                                                    \
    ((std::string)msg + ", LINE: " + (_next_token ? std::to_string(_next_token->line_number()) : "EOF"))
#else
#define PARSER_APPEND_LINE_NUM(msg)
#endif // DEBUG

#define PARSER_ACT_ELSE_RETURN(pred, action)                                                                           \
    PARSER_VERBOSE_ONLY(LOG("Current token: \"" + _next_token->value() + "\""););                                      \
    if (pred)                                                                                                          \
    {                                                                                                                  \
        action;                                                                                                        \
        PARSER_RETURN_IF_FALSE(_error.empty());                                                                        \
    }                                                                                                                  \
    else                                                                                                               \
        return nullptr;

#define PARSER_RETURN_IF_EOF()                                                                                         \
    if (!_next_token.has_value())                                                                                      \
    {                                                                                                                  \
        report_error();                                                                                                \
        return nullptr;                                                                                                \
    }

#define PARSER_ADVANCE_AND_RETURN_IF_EOF()                                                                             \
    advance_token();                                                                                                   \
    PARSER_RETURN_IF_EOF();

#define PARSER_ADVANCE_ELSE_RETURN(pred)                                                                               \
    PARSER_VERBOSE_ONLY(LOG("Current token: \"" + _next_token->value() + "\""););                                      \
    if (pred)                                                                                                          \
        advance_token();                                                                                               \
    else                                                                                                               \
        return nullptr;

#define PARSER_REPORT_AND_RETURN()                                                                                     \
    report_error();                                                                                                    \
    return nullptr;

#define PARSER_RETURN_IF_FALSE(pred)                                                                                   \
    if (!pred)                                                                                                         \
        return nullptr;

namespace ast
{
struct Class;
struct Feature;
struct Formal;
struct Expression;
struct Case;
struct Type;
struct ObjectExpression;
struct Program;
} // namespace ast

namespace parser
{
class Parser
{
  private:
    std::shared_ptr<lexer::Lexer> _lexer;
    std::optional<lexer::Token> _next_token;

    std::string _error; // error message

    inline void advance_token() { _next_token = std::move(_lexer->next()); }

    // error handling
    void report_error();
    // check type of the _next_token and report error if it has unexpected type
    bool check_next_and_report_error(const lexer::Token::TokenType &expected_type);

    // main parse methods
    std::shared_ptr<ast::Class> parse_class();
    std::shared_ptr<ast::Feature> parse_feature();
    std::shared_ptr<ast::Formal> parse_formal();
    std::shared_ptr<ast::Expression> parse_expr();

    // helper parse methods

    // parse using parse_func and push a result to container while _next_token is expected_lexeme or has expected_type
    template <class T>
    bool parse_list(std::vector<T> &container, std::function<T()> func, const lexer::Token::TokenType &expected_type,
                    bool skip_expected_token = false, const lexer::Token::TokenType &cutout_type = lexer::Token::ERROR);

    std::shared_ptr<ast::Expression> parse_if();
    std::shared_ptr<ast::Expression> parse_while();
    std::shared_ptr<ast::Expression> parse_case();
    std::shared_ptr<ast::Case> parse_one_case();
    std::shared_ptr<ast::Expression> parse_new();
    std::shared_ptr<ast::Expression> parse_isvoid();
    std::shared_ptr<ast::Expression> parse_not();
    std::shared_ptr<ast::Expression> parse_let();
    std::shared_ptr<ast::Expression> parse_let_define();
    // try to attach lhs to some operator after this expression
    std::shared_ptr<ast::Expression> parse_operators(const std::shared_ptr<ast::Expression> &lhs);
    std::shared_ptr<ast::Expression> parse_neg();
    std::shared_ptr<ast::Expression> parse_curly_brackets();
    std::shared_ptr<ast::Expression> parse_paren();
    // try to attach expr to some method dispatch after this expression
    std::shared_ptr<ast::Expression> parse_maybe_dispatch_or_oper(const std::shared_ptr<ast::Expression> &expr);
    bool parse_dispatch_list(std::vector<std::shared_ptr<ast::Expression>> &list);

    std::shared_ptr<ast::Type> parse_type();
    std::shared_ptr<ast::ObjectExpression> parse_object();
    // parse expressions starting from object
    std::shared_ptr<ast::Expression> parse_expr_object();
    std::shared_ptr<ast::Expression> parse_int();
    std::shared_ptr<ast::Expression> parse_string();
    std::shared_ptr<ast::Expression> parse_bool();

    // parse arithmetic or logical operators
    std::stack<int> _precedence_level; // prevent right recursion for left associative operators

    // operators precedence control
    int precedence_level(const lexer::Token::TokenType &type) const;
    inline void save_precedence_level() { _precedence_level.push(-1); }
    void restore_precedence_level();
    int current_precedence_level() const;
    void set_precedence_level(const int &lvl);

    bool token_is_left_assoc_operator() const;
    bool token_is_non_assoc_operator() const;
    template <class T> std::shared_ptr<ast::Expression> parse_operator(const std::shared_ptr<ast::Expression> &lhs);

    // create ast::Expression
    template <class T> std::shared_ptr<ast::Expression> make_expr(T &&variant, const int &line);

  public:
    /**
     * @brief Construct a new Parser
     *
     * @param lexer Lexer for retrieving tokens
     */
    explicit Parser(const std::shared_ptr<lexer::Lexer> &lexer) : _lexer(lexer), _next_token(_lexer->next())
    {
        _precedence_level.push(-1);
    }

    /**
     * @brief Do parse program
     *
     * @return AST for this program
     */
    std::shared_ptr<ast::Program> parse_program();

    /**
     * @brief Get the error message
     *
     * @return Error message
     */
    inline std::string error_msg() { return _error; }
};
} // namespace parser
