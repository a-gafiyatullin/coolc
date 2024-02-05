#pragma once

#include "parser/Parser.h"

using namespace parser;

template <class T> std::shared_ptr<ast::Expression> Parser::make_expr(T &&variant, const int &line)
{
    const auto expr = std::make_shared<ast::Expression>();
    expr->_data = std::forward<T>(variant);
    expr->_line_number = line;

    return expr;
}

template <class T>
bool Parser::parse_list(std::vector<T> &container, std::function<T()> func,
                        const lexer::Token::TokenType &expected_type, bool skip_expected_token,
                        const lexer::Token::TokenType &cutout_type)
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE_LIST")));

    const auto elem = func();
    if (elem == nullptr)
        return false;
    container.push_back(elem);

    while (_next_token.has_value() && _next_token->type() == expected_type)
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
        if (cutout_type == _next_token->type())
        {
            PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE_LIST BY CUTOUT")));
            return true;
        }

        const auto elem = func();
        if (elem == nullptr)
            return false;
        container.push_back(elem);
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE_LIST")));
    return true;
}

template <class T> std::shared_ptr<ast::Expression> Parser::parse_operator(const std::shared_ptr<ast::Expression> &lhs)
{
    const auto type = _next_token->type();

    if (precedence_level(type) <= current_precedence_level())
        return lhs;

    save_precedence_level();
    set_precedence_level(precedence_level(type));

    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE_OPERATOR")));
    std::shared_ptr<ast::Expression> res_expr = nullptr;

    const auto line = _next_token->line_number();

    PARSER_ADVANCE_AND_RETURN_IF_EOF();
    const auto rhs = parse_expr();
    PARSER_RETURN_IF_FALSE(!(rhs == nullptr));

    PARSER_RETURN_IF_EOF();
    restore_precedence_level();
    if (token_is_left_assoc_operator() || token_is_non_assoc_operator())
    {
        if (token_is_non_assoc_operator() && _next_token->same_token_type(type))
        {
            PARSER_REPORT_AND_RETURN();
        }
        PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE_OPERATOR FOR NEXT OPERATOR")));
        if (precedence_level(type) < precedence_level(_next_token->type()))
        {
            return make_expr(std::move(ast::BinaryExpression{T(), lhs, parse_operators(rhs)}), line);
        }

        return parse_operators(make_expr(std::move(ast::BinaryExpression{T(), lhs, rhs}), line));
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE_OPERATOR")));
    return make_expr(std::move(ast::BinaryExpression{T(), lhs, rhs}), line);
}