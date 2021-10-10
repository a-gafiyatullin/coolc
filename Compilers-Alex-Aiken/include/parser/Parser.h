#include "lexer/Lexer.h"
#include "ast/AST.h"

#ifdef PARSER_FULL_VERBOSE
#include <cassert>
#endif // PARSER_FULL_VERBOSE

#define act_else_return(pred, action)                                                             \
    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token.value().get_value() + "\"");); \
    if (pred)                                                                                     \
    {                                                                                             \
        action;                                                                                   \
        return_if_false(_error.empty());                                                          \
    }                                                                                             \
    else                                                                                          \
        return nullptr;

#define return_if_eof()           \
    if (!_next_token.has_value()) \
    {                             \
        report_error();           \
        return nullptr;           \
    }

#define advance_and_return_if_eof() \
    advance_token();                \
    return_if_eof();

#define advance_else_return(pred)                                                                 \
    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token.value().get_value() + "\"");); \
    if (pred)                                                                                     \
        advance_token();                                                                          \
    else                                                                                          \
        return nullptr;

#define report_and_return() \
    report_error();         \
    return nullptr;

#define return_if_false(pred) \
    if (!pred)                \
        return nullptr;

namespace parser
{
    class Parser
    {
    private:
        std::shared_ptr<lexer::Lexer> _lexer;
        std::optional<lexer::Token> _next_token;

        std::string _error; // error message

        inline void advance_token() { _next_token = _lexer->next(); }

        // error handling
        void report_error();
        // check type of the _next_token and report error if it has unexpected type
        bool check_next_and_report_error(const lexer::Token::TOKEN_TYPE &expected_type,
                                         const std::string &expected_lexeme = "");

        // main parse methods
        std::shared_ptr<ast::Class> parse_class();
        std::shared_ptr<ast::Feature> parse_feature();
        std::shared_ptr<ast::Formal> parse_formal();
        std::shared_ptr<ast::Expression> parse_expr();

        // helper parse methods

        // parse using parse_func and push a result to container while _next_token is expected_lexeme or has expected_type
        template <class T, T (Parser::*parse_func)()>
        bool parse_list(std::vector<T> &container, const lexer::Token::TOKEN_TYPE &expected_type,
                        const std::string &expected_lexeme = "", bool skip_expected_token = false,
                        const lexer::Token::TOKEN_TYPE &cutout_type = lexer::Token::ERROR, const std::string &cutout_lexeme = "");

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
        std::shared_ptr<ast::Expression> parse_brackets_and_neg();
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
        int _min_precedence; // prevent right recursion for left associative operators

        int precedence_level(const std::string &oper, const lexer::Token::TOKEN_TYPE &type) const;
        bool token_is_left_assoc_operator() const;
        bool token_is_non_assoc_operator() const;
        template <class T>
        std::shared_ptr<ast::Expression> parse_operator(const std::shared_ptr<ast::Expression> &lhs);

        // create ast::Expression
        template <class T>
        std::shared_ptr<ast::Expression> make_expr(T &&variant, const int &line);

        // for calls stack dump
#ifdef PARSER_FULL_VERBOSE
        int level = 0;

        void log(const std::string &msg) const;
        void log_enter(const std::string &msg);
        void log_exit(const std::string &msg);
#endif // PARSER_FULL_VERBOSE

    public:
        Parser(const std::shared_ptr<lexer::Lexer> &lexer) : _lexer(lexer), _next_token(_lexer->next()), _min_precedence(-1) {}

        std::shared_ptr<ast::Program> parse_program();
    };

    template <class T>
    std::shared_ptr<ast::Expression> Parser::make_expr(T &&variant, const int &line)
    {
        auto expr = std::make_shared<ast::Expression>();
        expr->_data = variant;
        PARSER_VERBOSE_ONLY(expr->_line_number = line);

        return expr;
    }

    template <class T, T (Parser::*parse_func)()>
    bool Parser::parse_list(std::vector<T> &container, const lexer::Token::TOKEN_TYPE &expected_type,
                            const std::string &expected_lexeme, bool skip_expected_token,
                            const lexer::Token::TOKEN_TYPE &cutout_type, const std::string &cutout_lexeme)
    {
        PARSER_FULL_VERBOSE_ONLY(log_enter("parse_list"));

        auto elem = (this->*parse_func)();
        if (elem == nullptr)
            return false;
        container.push_back(elem);

        while (_next_token.has_value() &&
               ((_next_token.value().get_type() == expected_type) &&
                (expected_lexeme == "" ||
                 _next_token.value().get_value() == expected_lexeme)))
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
            if (cutout_type != lexer::Token::ERROR && cutout_type == _next_token.value().get_type() ||
                cutout_lexeme != "" && cutout_lexeme == _next_token.value().get_value())
            {
                PARSER_FULL_VERBOSE_ONLY(log_exit("parse_list by cutout"));
                return true;
            }
            auto elem = (this->*parse_func)();
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
        auto oper = _next_token.value().get_value();
        auto type = _next_token.value().get_type();

        if (precedence_level(oper, type) <= _min_precedence)
            return lhs;

        int prev_precedence = _min_precedence;
        _min_precedence = precedence_level(oper, type);

        PARSER_FULL_VERBOSE_ONLY(log_enter("parse_operators"));
        std::shared_ptr<ast::Expression> res_expr = nullptr;

        PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

        advance_and_return_if_eof();
        auto rhs = parse_expr();
        return_if_false(!(rhs == nullptr));

        return_if_eof();
        _min_precedence = prev_precedence;
        if (token_is_left_assoc_operator() || token_is_non_assoc_operator())
        {
            if (token_is_non_assoc_operator() && _next_token.value().get_value() == oper)
            {
                report_and_return();
            }
            PARSER_FULL_VERBOSE_ONLY(log_exit("parse_operators for next operator"));
            if (precedence_level(oper, type) < precedence_level(_next_token.value().get_value(), _next_token.value().get_type()))
            {
                return make_expr(ast::BinaryExpression{T(), lhs, parse_operators(rhs)}, PARSER_VERBOSE_LINE(line));
            }
            else
            {
                return parse_operators(make_expr(ast::BinaryExpression{T(), lhs, rhs}, PARSER_VERBOSE_LINE(line)));
            }
        }

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_operators"));
        return make_expr(ast::BinaryExpression{T(), lhs, rhs}, PARSER_VERBOSE_LINE(line));
    }
}