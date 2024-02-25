#include "decls/Decls.h"
#include "parser/Parser.inline.h"
#include <functional>

using namespace parser;

// --------------------------------------- Error handling ---------------------------------------
void Parser::report_error()
{
    if (!_error.empty())
    {
        return;
    }

    if (!_next_token)
    {
        _error = "\"" + _lexer->file_name() + "\", line " + std::to_string(_lexer->line_number()) +
                 ": syntax error at or near EOF";
    }
    else
    {
        const auto &token = _next_token.value();
        const auto &type = token.type();

        _error = "\"" + _lexer->file_name() + "\", line " + std::to_string(token.line_number()) +
                 ": syntax error at or near ";
        if (type >= lexer::Token::SEMICOLON && type <= lexer::Token::COMMA)
        {
            _error += "\'" + token.value() + "\'";
        }
        else if (type >= lexer::Token::INT_CONST && type <= lexer::Token::STR_CONST)
        {
            _error += token.type_as_str() + " = " + token.value();
        }
        else
        {
            _error += token.type_as_str();
        }
    }

    PARSER_VERBOSE_ONLY(
        LOG("Actual token: \"" + _next_token->value() + "\", actual type = " + std::to_string(_next_token->type())));
}

bool Parser::check_next_and_report_error(const lexer::Token::TokenType &expected_type)
{
    if (!_error.empty() || !_next_token || !_next_token->same_token_type(expected_type))
    {
        report_error();
        return false;
    }

    return true;
}

// --------------------------------------- Main parse methods ---------------------------------------
std::shared_ptr<ast::Program> Parser::parse_program()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE PROGRAM")));

    // check if program is empty
    PARSER_RETURN_IF_FALSE(_next_token);

    const auto program = std::make_shared<ast::Program>();
    program->_line_number = _next_token->line_number();

    const bool result = parse_list<std::shared_ptr<ast::Class>>(
        program->_classes, std::bind(&Parser::parse_class, this), lexer::Token::CLASS);
    PARSER_RETURN_IF_FALSE(result);
    if (_next_token)
    {
        PARSER_REPORT_AND_RETURN();
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE PROGRAM")));
    return program;
}

std::shared_ptr<ast::Class> Parser::parse_class()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE CLASS")));

    const auto klass = std::make_shared<ast::Class>();
    klass->_line_number = _next_token->line_number();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::CLASS));
    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), klass->_type = parse_type());

    PARSER_RETURN_IF_EOF();
    if (_next_token->same_token_type(lexer::Token::INHERITS))
    {
        PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::INHERITS));
        PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), klass->_parent = parse_type());
    }
    else
    {
        klass->_parent = std::make_shared<ast::Type>();
        klass->_parent->_string = BaseClassesNames[BaseClasses::OBJECT];
    }

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::LEFT_CURLY_BRACKET));
    if (!_next_token->same_token_type(lexer::Token::RIGHT_CURLY_BRACKET))
    {
        const bool result = parse_list<std::shared_ptr<ast::Feature>>(
            klass->_features, std::bind(&Parser::parse_feature, this), lexer::Token::OBJECTID);
        PARSER_RETURN_IF_FALSE(result);
    }
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::RIGHT_CURLY_BRACKET));
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::SEMICOLON));

    klass->_file_name = _lexer->file_name();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE CLASS")));
    return klass;
}

std::shared_ptr<ast::Feature> Parser::parse_feature()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE FEATURE")));

    const auto feature = std::make_shared<ast::Feature>();
    feature->_line_number = _next_token->line_number();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), feature->_object = parse_object());

    PARSER_RETURN_IF_EOF();
    // check if feature is method
    if (_next_token->same_token_type(lexer::Token::LEFT_PAREN))
    {
        feature->_base = ast::MethodFeature();
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        // parse arguments
        if (!_next_token->same_token_type(lexer::Token::RIGHT_PAREN))
        {
            const bool result = parse_list<std::shared_ptr<ast::Formal>>(
                std::get<ast::MethodFeature>(feature->_base)._formals, std::bind(&Parser::parse_formal, this),
                lexer::Token::OBJECTID);
            PARSER_RETURN_IF_FALSE(result);
        }
        PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::RIGHT_PAREN));
    }

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::COLON));
    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), feature->_type = parse_type());

    PARSER_RETURN_IF_EOF();
    switch (_next_token->type())
    {
    case lexer::Token::LEFT_CURLY_BRACKET: {
        // parse method body
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        PARSER_ACT_ELSE_RETURN(true, feature->_expr = parse_expr());
        PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::RIGHT_CURLY_BRACKET));
        break;
    }
    case lexer::Token::ASSIGN: {
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        PARSER_ACT_ELSE_RETURN(true, feature->_expr = parse_expr());
        break;
    }
    }
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::SEMICOLON));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE FEATURE")));
    return feature;
}

std::shared_ptr<ast::Formal> Parser::parse_formal()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE FORMAL")));

    const auto formal = std::make_shared<ast::Formal>();
    formal->_line_number = _next_token->line_number();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), formal->_object = parse_object());
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::COLON));
    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), formal->_type = parse_type());

    PARSER_RETURN_IF_EOF();
    // skip ',' if it is list of formals
    if (_next_token->same_token_type(lexer::Token::COMMA))
    {
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE FORMAL")));
    return formal;
}

std::shared_ptr<ast::Expression> Parser::parse_expr()
{
    std::shared_ptr<ast::Expression> expr = nullptr;

    save_precedence_level();

    switch (_next_token->type())
    {
    case lexer::Token::OBJECTID: {
        expr = parse_expr_object();
        break;
    }
    case lexer::Token::INT_CONST: {
        expr = parse_int();
        break;
    }
    case lexer::Token::STR_CONST: {
        expr = parse_string();
        break;
    }
    case lexer::Token::BOOL_CONST: {
        expr = parse_bool();
        break;
    }
    case lexer::Token::NOT: {
        expr = parse_not();
        break;
    }
    case lexer::Token::IF: {
        expr = parse_if();
        break;
    }
    case lexer::Token::WHILE: {
        expr = parse_while();
        break;
    }
    case lexer::Token::NEG: {
        expr = parse_neg();
        break;
    }
    case lexer::Token::LEFT_CURLY_BRACKET: {
        expr = parse_curly_brackets();
        break;
    }
    case lexer::Token::LEFT_PAREN: {
        expr = parse_paren();
        break;
    }
    case lexer::Token::LET: {
        expr = parse_let();
        break;
    }
    case lexer::Token::CASE: {
        expr = parse_case();
        break;
    }
    case lexer::Token::NEW: {
        expr = parse_new();
        break;
    }
    case lexer::Token::ISVOID: {
        expr = parse_isvoid();
        break;
    }
    default:
        PARSER_REPORT_AND_RETURN();
    }

    restore_precedence_level();

    return expr == nullptr ? expr : parse_maybe_dispatch_or_oper(expr);
}

// --------------------------------------- Helper parse methods ---------------------------------------
std::shared_ptr<ast::Expression> Parser::parse_if()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE IF")));
    const auto line = _next_token->line_number();

    PARSER_ADVANCE_AND_RETURN_IF_EOF();
    const auto predicate = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::THEN));
    const auto true_path = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::ELSE));
    const auto false_path = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::FI));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE IF")));
    return make_expr(std::move(ast::IfExpression{predicate, true_path, false_path}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_while()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE WHILE")));
    const auto line = _next_token->line_number();

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    const auto predicate = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::LOOP));
    const auto body = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::POOL));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE WHILE")));
    return make_expr(std::move(ast::WhileExpression{predicate, body}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_case()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE CASE")));
    const auto line = _next_token->line_number();
    ast::CaseExpression case_expr;

    PARSER_ADVANCE_AND_RETURN_IF_EOF();
    case_expr._expr = parse_expr();

    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::OF));
    const bool result = parse_list<std::shared_ptr<ast::Case>>(
        case_expr._cases, std::bind(&Parser::parse_one_case, this), lexer::Token::SEMICOLON, true, lexer::Token::ESAC);
    PARSER_RETURN_IF_FALSE(result);
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::ESAC));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE CASE")));
    return make_expr(std::move(case_expr), line);
}

std::shared_ptr<ast::Expression> Parser::parse_new()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE NEW")));
    const auto line = _next_token->line_number();

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE NEW")));
    return make_expr(std::move(ast::NewExpression{parse_type()}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_isvoid()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE ISVOID")));
    const auto line = _next_token->line_number();
    set_precedence_level(precedence_level(lexer::Token::ISVOID));

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE ISVOID")));
    return make_expr(std::move(ast::UnaryExpression{ast::IsVoidExpression(), parse_expr()}), line);
}

std::shared_ptr<ast::Case> Parser::parse_one_case()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE ONE CASE")));
    const auto kase = std::make_shared<ast::Case>();
    kase->_line_number = _next_token->line_number();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), kase->_object = parse_object());
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::COLON));
    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), kase->_type = parse_type());
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::DARROW));
    PARSER_ACT_ELSE_RETURN(true, kase->_expr = parse_expr());

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE ONE CASE")));
    return kase;
}

std::shared_ptr<ast::Expression> Parser::parse_not()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE NOT")));
    const auto line = _next_token->line_number();
    set_precedence_level(precedence_level(lexer::Token::NOT));

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE NOT")));
    return make_expr(std::move(ast::UnaryExpression{ast::NotExpression(), parse_expr()}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_let()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE LET")));

    PARSER_RETURN_IF_FALSE(check_next_and_report_error(lexer::Token::LET));
    const auto expr = parse_let_define();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE LET")));
    return expr;
}

std::shared_ptr<ast::Expression> Parser::parse_let_define()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE LET DEFINE")));
    ast::LetExpression def;

    PARSER_ADVANCE_AND_RETURN_IF_EOF(); // skip "LET" or ","
    const auto line = _next_token->line_number();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), def._object = parse_object());
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::COLON));
    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), def._type = parse_type());

    PARSER_RETURN_IF_EOF();
    if (_next_token->same_token_type(lexer::Token::ASSIGN))
    {
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        PARSER_ACT_ELSE_RETURN(true, def._expr = parse_expr());
    }

    if (_next_token->same_token_type(lexer::Token::IN))
    {
        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        PARSER_ACT_ELSE_RETURN(true, def._body_expr = parse_expr());

        PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE LET DEFINE FOR IN")));
        return make_expr(std::move(def), line);
    }

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::COMMA), def._body_expr = parse_let_define());

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE LET DEFINE")));
    return make_expr(std::move(def), line);
}

std::shared_ptr<ast::Expression> Parser::parse_neg()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE NEG")));
    const auto line = _next_token->line_number();
    set_precedence_level(precedence_level(lexer::Token::NEG));

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    const auto expr = parse_expr();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE NEG")));
    return make_expr(std::move(ast::UnaryExpression{ast::NegExpression(), expr}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_paren()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE PAREN")));

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    const auto expr = parse_expr();
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::RIGHT_PAREN));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE PAREN")));
    return expr;
}

std::shared_ptr<ast::Expression> Parser::parse_curly_brackets()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE BRACKETS")));
    const auto line = _next_token->line_number();

    // parse block
    ast::ListExpression expr_list;

    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    const bool result =
        parse_list<std::shared_ptr<ast::Expression>>(expr_list._exprs, std::bind(&Parser::parse_expr, this),
                                                     lexer::Token::SEMICOLON, true, lexer::Token::RIGHT_CURLY_BRACKET);
    PARSER_RETURN_IF_FALSE(result);
    PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::RIGHT_CURLY_BRACKET));

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE BRACKETS")));
    return make_expr(std::move(expr_list), line);
}

bool Parser::parse_dispatch_list(std::vector<std::shared_ptr<ast::Expression>> &list)
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE DISPATCH LIST")));

    if (check_next_and_report_error(lexer::Token::LEFT_PAREN))
    {
        advance_token();
    }
    else
    {
        return false;
    }

    if (!_next_token->same_token_type(lexer::Token::RIGHT_PAREN))
    {
        const auto result = parse_list<std::shared_ptr<ast::Expression>>(list, std::bind(&Parser::parse_expr, this),
                                                                         lexer::Token::COMMA, true);
        if (!result)
        {
            return false;
        }
    }
    PARSER_VERBOSE_ONLY(LOG("Current token: \"" + _next_token->value() + "\""););
    if (check_next_and_report_error(lexer::Token::RIGHT_PAREN))
    {
        advance_token();
    }
    else
    {
        return false;
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE DISPATCH LIST")));
    return true;
}
// --------------------------------------- Parse arithmetic or logical operators ---------------------------------------
bool Parser::token_is_left_assoc_operator() const
{
    const auto &type = _next_token->type();
    return _next_token->same_token_type(lexer::Token::LESS) || _next_token->same_token_type(lexer::Token::PLUS) ||
           _next_token->same_token_type(lexer::Token::MINUS) || _next_token->same_token_type(lexer::Token::ASTERISK) ||
           _next_token->same_token_type(lexer::Token::SLASH);
}

bool Parser::token_is_non_assoc_operator() const
{
    return _next_token->same_token_type(lexer::Token::EQUALS) || _next_token->same_token_type(lexer::Token::LE);
}

int Parser::precedence_level(const lexer::Token::TokenType &type) const
{
    static const std::unordered_map<lexer::Token::TokenType, int> PRECEDENCE{
        {lexer::Token::ASSIGN, 0}, {lexer::Token::NOT, 1},  {lexer::Token::LESS, 2},  {lexer::Token::EQUALS, 3},
        {lexer::Token::LE, 3},     {lexer::Token::PLUS, 4}, {lexer::Token::MINUS, 4}, {lexer::Token::ASTERISK, 5},
        {lexer::Token::SLASH, 5},  {lexer::Token::NEG, 6},  {lexer::Token::ISVOID, 7}};

    return PRECEDENCE.at(type);
}

void Parser::restore_precedence_level()
{
    GUARANTEE_DEBUG(!_precedence_level.empty());
    _precedence_level.pop();
}

int Parser::current_precedence_level() const
{
    GUARANTEE_DEBUG(!_precedence_level.empty());
    return _precedence_level.top();
}

void Parser::set_precedence_level(const int &lvl)
{
    GUARANTEE_DEBUG(!_precedence_level.empty());
    _precedence_level.top() = lvl;
}

std::shared_ptr<ast::Expression> Parser::parse_operators(const std::shared_ptr<ast::Expression> &lhs)
{
    if (_next_token->same_token_type(lexer::Token::PLUS))
    {
        return parse_operator<ast::PlusExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::MINUS))
    {
        return parse_operator<ast::MinusExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::ASTERISK))
    {
        return parse_operator<ast::MulExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::SLASH))
    {
        return parse_operator<ast::DivExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::LESS))
    {
        return parse_operator<ast::LTExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::EQUALS))
    {
        return parse_operator<ast::EqExpression>(lhs);
    }

    if (_next_token->same_token_type(lexer::Token::LE))
    {
        return parse_operator<ast::LEExpression>(lhs);
    }

    return lhs;
}

// ----------------- Methods for parsing expression with complex type of starting -----------------
std::shared_ptr<ast::Type> Parser::parse_type()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE TYPE")));

    const auto type = std::make_shared<ast::Type>();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID), type->_string = _next_token->value());
    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE TYPE")));
    return type;
}

std::shared_ptr<ast::ObjectExpression> Parser::parse_object()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE OBJECT")));

    const auto obj = std::make_shared<ast::ObjectExpression>();

    PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), obj->_object = _next_token->value());
    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE OBJECT")));
    return obj;
}

std::shared_ptr<ast::Expression> Parser::parse_expr_object()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE EXPR OBJECT")));
    const auto line = _next_token->line_number();

    const auto lhs = parse_object();

    switch (_next_token->type())
    {
    case lexer::Token::ASSIGN: {
        PARSER_ADVANCE_AND_RETURN_IF_EOF();

        PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE EXPR OBJECT FOR ASSIGN")));
        return make_expr(std::move(ast::AssignExpression{lhs, parse_expr()}), line);
    }
    case lexer::Token::LEFT_PAREN: {
        ast::DispatchExpression dispatch;
        dispatch._expr = make_expr(std::move(ast::ObjectExpression{SelfObject}), line);
        dispatch._object = lhs;

        PARSER_RETURN_IF_FALSE(parse_dispatch_list(dispatch._args));

        PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE EXPR_OBJECT FOR (")));
        return make_expr(std::move(dispatch), line);
    }
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE EXPR OBJECT")));
    return make_expr(std::move(*lhs), line);
}

std::shared_ptr<ast::Expression> Parser::parse_int()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE INT")));
    const auto line = _next_token->line_number();

    const auto value = std::stoi(_next_token->value());
    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE INT")));
    return make_expr(std::move(ast::IntExpression{value}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_string()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE STRING")));
    const auto line = _next_token->line_number();

    const auto value = _next_token->value();
    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE STRING")));
    return make_expr(std::move(ast::StringExpression{value}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_bool()
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE BOOL")));
    const auto line = _next_token->line_number();

    const auto value = _next_token->value() == "true"; // TODO: make constant
    PARSER_ADVANCE_AND_RETURN_IF_EOF();

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE BOOL")));
    return make_expr(std::move(ast::BoolExpression{value}), line);
}

std::shared_ptr<ast::Expression> Parser::parse_maybe_dispatch_or_oper(const std::shared_ptr<ast::Expression> &expr)
{
    PARSER_VERBOSE_ONLY(LOG_ENTER(PARSER_APPEND_LINE_NUM("PARSE MAYBE DISPATCH OR OPER")));
    const auto line = _next_token->line_number();

    ast::DispatchExpression dispatch;
    dispatch._expr = expr;
    auto is_dispatch = false;

    const auto &oper = _next_token->value();
    if (_next_token->same_token_type(lexer::Token::AT))
    {
        is_dispatch = true;
        dispatch._base = ast::StaticDispatchExpression();

        PARSER_ADVANCE_AND_RETURN_IF_EOF();
        PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::TYPEID),
                               std::get<ast::StaticDispatchExpression>(dispatch._base)._type = parse_type());
    }
    else if (_next_token->same_token_type(lexer::Token::DOT))
    {
        is_dispatch = true;
        dispatch._base = ast::VirtualDispatchExpression();
    }

    if (is_dispatch)
    {
        PARSER_ADVANCE_ELSE_RETURN(check_next_and_report_error(lexer::Token::DOT));
        PARSER_ACT_ELSE_RETURN(check_next_and_report_error(lexer::Token::OBJECTID), dispatch._object = parse_object());

        PARSER_RETURN_IF_FALSE(parse_dispatch_list(dispatch._args));

        PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE MAYBE DISPATCH OR OPER FOR DISPATCH")));
        return parse_maybe_dispatch_or_oper(make_expr(std::move(dispatch), line));
    }

    PARSER_VERBOSE_ONLY(LOG_EXIT(PARSER_APPEND_LINE_NUM("PARSE MAYBE DISPATCH OR OPER")));
    return parse_operators(expr);
}
