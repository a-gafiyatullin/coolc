#pragma once

#include "lexer/Lexer.h"
#include "ast/AST.h"
#include <stack>
#include <functional>

#ifdef PARSER_FULL_VERBOSE
#include <cassert>
#endif // PARSER_FULL_VERBOSE

#define PARSER_ACT_ELSE_RETURN(pred, action)                                               \
    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token->get_value() + "\"");); \
    if (pred)                                                                              \
    {                                                                                      \
        action;                                                                            \
        PARSER_RETURN_IF_FALSE(_error.empty());                                            \
    }                                                                                      \
    else                                                                                   \
        return nullptr;

#define PARSER_RETURN_IF_EOF()    \
    if (!_next_token.has_value()) \
    {                             \
        report_error();           \
        return nullptr;           \
    }

#define PARSER_ADVANCE_AND_RETURN_IF_EOF() \
    advance_token();                       \
    PARSER_RETURN_IF_EOF();

#define PARSER_ADVANCE_ELSE_RETURN(pred)                                                   \
    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token->get_value() + "\"");); \
    if (pred)                                                                              \
        advance_token();                                                                   \
    else                                                                                   \
        return nullptr;

#define PARSER_REPORT_AND_RETURN() \
    report_error();                \
    return nullptr;

#define PARSER_RETURN_IF_FALSE(pred) \
    if (!pred)                       \
        return nullptr;

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
        bool check_next_and_report_error(const lexer::Token::TOKEN_TYPE &expected_type);

        // main parse methods
        std::shared_ptr<ast::Class> parse_class();
        std::shared_ptr<ast::Feature> parse_feature();
        std::shared_ptr<ast::Formal> parse_formal();
        std::shared_ptr<ast::Expression> parse_expr();

        // helper parse methods

        // parse using parse_func and push a result to container while _next_token is expected_lexeme or has expected_type
        template <class T>
        bool parse_list(std::vector<T> &container, std::function<T()> func, const lexer::Token::TOKEN_TYPE &expected_type,
                        bool skip_expected_token = false, const lexer::Token::TOKEN_TYPE &cutout_type = lexer::Token::ERROR);

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
        int precedence_level(const lexer::Token::TOKEN_TYPE &type) const;
        inline void save_precedence_level() { _precedence_level.push(-1); };
        void restore_precedence_level();
        int current_precedence_level() const;
        void set_precedence_level(const int &lvl);

        bool token_is_left_assoc_operator() const;
        bool token_is_non_assoc_operator() const;
        template <class T>
        std::shared_ptr<ast::Expression> parse_operator(const std::shared_ptr<ast::Expression> &lhs);

        // create ast::Expression
        template <class T>
        std::shared_ptr<ast::Expression> make_expr(T &&variant, const int &line);

        // for calls stack dump
#ifdef PARSER_FULL_VERBOSE
        int _level = 0;

        void log(const std::string &msg) const;
        void log_enter(const std::string &msg);
        void log_exit(const std::string &msg);
#endif // PARSER_FULL_VERBOSE

    public:
        explicit Parser(const std::shared_ptr<lexer::Lexer> &lexer) : _lexer(lexer), _next_token(_lexer->next()) { _precedence_level.push(-1); }

        std::shared_ptr<ast::Program> parse_program();
        inline std::string get_error_msg() { return _error; }
    };

    template <class T>
    std::shared_ptr<ast::Expression> Parser::make_expr(T &&variant, const int &line)
    {
        auto expr = std::make_shared<ast::Expression>();
        expr->_data = std::forward<T>(variant);
        PARSER_VERBOSE_ONLY(expr->_line_number = line);

        return expr;
    }

    template <class T>
    bool Parser::parse_list(std::vector<T> &container, std::function<T()> func, const lexer::Token::TOKEN_TYPE &expected_type,
                            bool skip_expected_token, const lexer::Token::TOKEN_TYPE &cutout_type)
    {
        PARSER_FULL_VERBOSE_ONLY(log_enter("parse_list"));

        auto elem = func();
        if (elem == nullptr)
            return false;
        container.push_back(elem);

        while (_next_token.has_value() && _next_token->get_type() == expected_type)
        {
            if (skip_expected_token)
            {
                advance_token();
                if (!_next_token.has_value())
                {
                    return false;
                }
            }
            // should we parse further?
            if (cutout_type == _next_token->get_type())
            {
                PARSER_FULL_VERBOSE_ONLY(log_exit("parse_list by cutout"));
                return true;
            }

            auto elem = func();
            if (elem == nullptr)
                return false;
            container.push_back(elem);
        }

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_list"));
        return true;
    }

    template <class T>
    std::shared_ptr<ast::Expression> Parser::parse_operator(const std::shared_ptr<ast::Expression> &lhs)
    {
        auto type = _next_token->get_type();

        if (precedence_level(type) <= current_precedence_level())
            return lhs;

        save_precedence_level();
        set_precedence_level(precedence_level(type));

        PARSER_FULL_VERBOSE_ONLY(log_enter("parse_operators"));
        std::shared_ptr<ast::Expression> res_expr = nullptr;

        PARSER_VERBOSE_ONLY(int line = _next_token->get_line_number(););

        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        auto rhs = parse_expr();
        PARSER_RETURN_IF_FALSE(!(rhs == nullptr));

        PARSER_RETURN_IF_EOF();
        restore_precedence_level();
        if (token_is_left_assoc_operator() || token_is_non_assoc_operator())
        {
            if (token_is_non_assoc_operator() && _next_token->same_token_type(type))
            {
                PARSER_REPORT_AND_RETURN();
            }
            PARSER_FULL_VERBOSE_ONLY(log_exit("parse_operators for next operator"));
            if (precedence_level(type) < precedence_level(_next_token->get_type()))
            {
                return make_expr(std::move(ast::BinaryExpression{T(), lhs, parse_operators(rhs)}), PARSER_VERBOSE_LINE(line));
            }
            else
            {
                return parse_operators(make_expr(std::move(ast::BinaryExpression{T(), lhs, rhs}), PARSER_VERBOSE_LINE(line)));
            }
        }

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_operators"));
        return make_expr(std::move(ast::BinaryExpression{T(), lhs, rhs}), PARSER_VERBOSE_LINE(line));
    }
}