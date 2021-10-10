#include "parser/Parser.h"

using namespace parser;

#ifdef PARSER_FULL_VERBOSE
void Parser::log(const std::string &msg) const
{
    std::cout << std::string(level, ' ') << msg << std::endl;
}

void Parser::log_enter(const std::string &msg)
{
    log(std::string("Enter ") + msg);
    level += 4;
}

void Parser::log_exit(const std::string &msg)
{
    level -= 4;
    log(std::string("Exit ") + msg);
}
#endif // PARSER_FULL_VERBOSE

// --------------------------------------- Error handling ---------------------------------------
void Parser::report_error()
{
    if (!_error.empty())
    {
        return;
    }

    if (!_next_token.has_value())
    {
        _error = "\"" + _lexer->get_file_name() + "\", line " + std::to_string(_lexer->get_line_number()) + ": syntax error at or near EOF";
    }
    else
    {
        auto token = _next_token.value();
        auto type = token.get_type();
        _error = "\"" + _lexer->get_file_name() + "\", line " + std::to_string(token.get_line_number()) + ": syntax error at or near ";
        if (type == lexer::Token::OPERATIONS_AND_CONTROLS)
        {
            _error += "\'" + token.get_value() + "\'";
        }
        else if (type >= lexer::Token::INT_CONST && type <= lexer::Token::STR_CONST)
        {
            _error += token.get_type_as_str() + " = " + token.get_value();
        }
        else
        {
            _error += token.get_type_as_str();
        }
    }

    std::cout << _error << std::endl;
    PARSER_FULL_VERBOSE_ONLY(log("Actual token: " + _next_token.value().get_value()));
}

bool Parser::check_next_and_report_error(const lexer::Token::TOKEN_TYPE &expected_type,
                                         const std::string &expected_lexeme)
{
    if (!_error.empty() || !_next_token.has_value() ||
        _next_token.value().get_type() != expected_type ||
        (expected_lexeme != "" && _next_token.value().get_value() != expected_lexeme))
    {
        report_error();
        return false;
    }

    return true;
}

// --------------------------------------- Main parse methods ---------------------------------------
std::shared_ptr<ast::Program> Parser::parse_program()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_program"));

    // check if program is empty
    return_if_false(_next_token.has_value());

    auto program = std::make_shared<ast::Program>();
    PARSER_VERBOSE_ONLY(program->_line_number = _next_token.value().get_line_number());

    bool result = parse_list<std::shared_ptr<ast::Class>, &Parser::parse_class>(program->_classes, lexer::Token::CLASS);
    return_if_false(result);
    if (_next_token.has_value())
    {
        report_and_return();
    }

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_program"));
    return program;
}

std::shared_ptr<ast::Class> Parser::parse_class()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_class"));

    auto class_ = std::make_shared<ast::Class>();
    PARSER_VERBOSE_ONLY(class_->_line_number = _next_token.value().get_line_number());

    advance_else_return(check_next_and_report_error(lexer::Token::CLASS));
    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    class_->_type = parse_type());

    return_if_eof();
    if (_next_token.value().get_type() == lexer::Token::INHERITS)
    {
        advance_else_return(check_next_and_report_error(lexer::Token::INHERITS));
        act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                        class_->_parent = parse_type());
    }
    else
    {
        class_->_parent = std::make_shared<ast::Type>();
        class_->_parent->_string = "Object";
    }

    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "{"));
    bool result = parse_list<std::shared_ptr<ast::Feature>, &Parser::parse_feature>(class_->_features, lexer::Token::OBJECTID);
    return_if_false(result);
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "}"));
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ";"));

    PARSER_VERBOSE_ONLY(class_->_file_name = _lexer->get_file_name());

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_class"));
    return class_;
}

std::shared_ptr<ast::Feature> Parser::parse_feature()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_feature"));

    auto feature = std::make_shared<ast::Feature>();
    PARSER_VERBOSE_ONLY(feature->_line_number = _next_token.value().get_line_number());

    act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                    feature->_object = parse_object());

    return_if_eof();
    // check if feature is method
    if (_next_token.value().get_value() == "(")
    {
        feature->_base = ast::MethodFeature();
        advance_and_return_if_eof();
        // parse arguments
        if (_next_token.value().get_value() != ")")
        {
            bool result = parse_list<std::shared_ptr<ast::Formal>,
                                     &Parser::parse_formal>(std::get<ast::MethodFeature>(feature->_base)._formals, lexer::Token::OBJECTID);
            return_if_false(result);
        }
        advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ")"));
    }

    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ":"));
    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    feature->_type = parse_type());

    return_if_eof();
    auto token = _next_token.value();
    switch (token.get_type())
    {
    case lexer::Token::OPERATIONS_AND_CONTROLS:
    {
        // parse method body
        if (token.get_value() == "{")
        {
            advance_and_return_if_eof();
            act_else_return(true, feature->_expr = parse_expr());
            advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "}"));
        }
        break;
    }
    case lexer::Token::ASSIGN:
    {
        advance_and_return_if_eof();
        act_else_return(true, feature->_expr = parse_expr());
        break;
    }
    }
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ";"));

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_feature"));
    return feature;
}

std::shared_ptr<ast::Formal> Parser::parse_formal()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_formal"));

    auto formal = std::make_shared<ast::Formal>();
    PARSER_VERBOSE_ONLY(formal->_line_number = _next_token.value().get_line_number());

    act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                    formal->_object = parse_object());
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ":"));
    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    formal->_type = parse_type());

    return_if_eof();
    // skip ',' if it is list of formals
    if (_next_token.value().get_value() == ",")
    {
        advance_and_return_if_eof();
    }

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_formal"));
    return formal;
}

std::shared_ptr<ast::Expression> Parser::parse_expr()
{
    std::shared_ptr<ast::Expression> expr = nullptr;

    switch (_next_token.value().get_type())
    {
    case lexer::Token::OBJECTID:
    {
        expr = parse_expr_object();
        break;
    }
    case lexer::Token::INT_CONST:
    {
        expr = parse_int();
        break;
    }
    case lexer::Token::STR_CONST:
    {
        expr = parse_string();
        break;
    }
    case lexer::Token::BOOL_CONST:
    {
        expr = parse_bool();
        break;
    }
    case lexer::Token::NOT:
    {
        int prev_precedence = _min_precedence;
        _min_precedence = precedence_level("", lexer::Token::NOT);

        expr = parse_not();

        _min_precedence = prev_precedence;
        break;
    }
    case lexer::Token::IF:
    {
        expr = parse_if();
        break;
    }
    case lexer::Token::WHILE:
    {
        expr = parse_while();
        break;
    }
    case lexer::Token::OPERATIONS_AND_CONTROLS:
    {
        expr = parse_brackets_and_neg();
        break;
    }
    case lexer::Token::LET:
    {
        expr = parse_let();
        break;
    }
    case lexer::Token::CASE:
    {
        expr = parse_case();
        break;
    }
    case lexer::Token::NEW:
    {
        expr = parse_new();
        break;
    }
    case lexer::Token::ISVOID:
    {
        int prev_precedence = _min_precedence;
        _min_precedence = precedence_level("", lexer::Token::ISVOID);

        expr = parse_isvoid();

        _min_precedence = prev_precedence;
        break;
    }
    default:
        report_and_return();
    }

    return expr == nullptr ? expr : parse_maybe_dispatch_or_oper(expr);
}

// --------------------------------------- Helper parse methods ---------------------------------------
std::shared_ptr<ast::Expression> Parser::parse_if()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_if"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    advance_and_return_if_eof();
    auto predicate = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::THEN));
    auto true_path = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::ELSE));
    auto false_path = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::FI));

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_if"));
    return make_expr(ast::IfExpression{predicate, true_path, false_path}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_while()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_while"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    advance_and_return_if_eof();

    auto predicate = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::LOOP));
    auto body = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::POOL));

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_while"));
    return make_expr(ast::WhileExpression{predicate, body}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_case()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_case"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););
    ast::CaseExpression case_expr;

    advance_and_return_if_eof();
    case_expr._expr = parse_expr();

    advance_else_return(check_next_and_report_error(lexer::Token::OF));
    bool result = parse_list<std::shared_ptr<ast::Case>, &Parser::parse_one_case>(case_expr._cases,
                                                                                  lexer::Token::OPERATIONS_AND_CONTROLS, ";",
                                                                                  true, lexer::Token::ESAC);
    return_if_false(result);
    advance_else_return(check_next_and_report_error(lexer::Token::ESAC));

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_case"));
    return make_expr(case_expr, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_new()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_new"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_new"));
    return make_expr(ast::NewExpression{parse_type()}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_isvoid()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_isvoid"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_isvoid"));
    return make_expr(ast::UnaryExpression{ast::IsVoidExpression(), parse_expr()}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Case> Parser::parse_one_case()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_one_case"));
    auto case_ = std::make_shared<ast::Case>();
    PARSER_VERBOSE_ONLY(case_->_line_number = _next_token.value().get_line_number());

    act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                    case_->_object = parse_object());
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ":"));
    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    case_->_type = parse_type());
    advance_else_return(check_next_and_report_error(lexer::Token::DARROW));
    act_else_return(true, case_->_expr = parse_expr());

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_one_case"));
    return case_;
}

std::shared_ptr<ast::Expression> Parser::parse_not()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_not"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_not"));
    return make_expr(ast::UnaryExpression{ast::NotExpression(), parse_expr()}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_let()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_let"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    return_if_false(check_next_and_report_error(lexer::Token::LET));

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_let"));
    auto expr = parse_let_define();
    PARSER_VERBOSE_ONLY(if (expr != nullptr) expr->_line_number = line;);

    return expr;
}

std::shared_ptr<ast::Expression> Parser::parse_let_define()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_let_define"));
    ast::LetExpression def;

    advance_and_return_if_eof(); // skip "LET" or ","
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                    def._object = parse_object());
    advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ":"));
    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    def._type = parse_type());

    return_if_eof();
    if (_next_token.value().get_type() == lexer::Token::ASSIGN)
    {
        advance_and_return_if_eof();
        act_else_return(true, def._expr = parse_expr());
    }

    if (_next_token.value().get_type() == lexer::Token::IN)
    {
        advance_and_return_if_eof();
        act_else_return(true, def._body_expr = parse_expr());

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_let_define for IN"));
        return make_expr(def, PARSER_VERBOSE_LINE(line));
    }

    act_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ","),
                    def._body_expr = parse_let_define());

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_let_define"));
    return make_expr(def, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_brackets_and_neg()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_brackets_and_neg"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    auto oper = _next_token.value().get_value();
    if (oper == "~")
    {
        advance_and_return_if_eof();

        int prev_precedence = _min_precedence;
        _min_precedence = precedence_level("~", lexer::Token::OPERATIONS_AND_CONTROLS);

        auto expr = parse_expr();

        _min_precedence = prev_precedence;
        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_brackets_and_neg for ~"));
        return make_expr(ast::UnaryExpression{ast::NegExpression(), expr}, PARSER_VERBOSE_LINE(line));
    }
    // parse block
    else if (oper == "{")
    {
        ast::ListExpression expr_list;

        advance_and_return_if_eof();

        bool result = parse_list<std::shared_ptr<ast::Expression>, &Parser::parse_expr>(expr_list._exprs,
                                                                                        lexer::Token::OPERATIONS_AND_CONTROLS, ";",
                                                                                        true, lexer::Token::ERROR, "}");
        return_if_false(result);
        advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "}"));

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_brackets_and_neg for {"));
        return make_expr(expr_list, PARSER_VERBOSE_LINE(line));
    }
    else if (oper == "(")
    {
        advance_and_return_if_eof();

        _min_precedence = -1;
        auto expr = parse_expr();
        advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ")"));

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_brackets_and_neg for ("));
        return expr;
    }
    else
    {
        report_and_return();
    }
}

bool Parser::parse_dispatch_list(std::vector<std::shared_ptr<ast::Expression>> &list)
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_dispatch_list"));

    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token.value().get_value() + "\""););
    if (check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "("))
    {
        advance_token();
    }
    else
    {
        return false;
    }

    if (_next_token.value().get_value() != ")")
    {
        // one argument before the first comma
        bool result = parse_list<std::shared_ptr<ast::Expression>, &Parser::parse_expr>(list,
                                                                                        lexer::Token::OPERATIONS_AND_CONTROLS, ",",
                                                                                        true);
        if (!result)
        {
            return false;
        }
    }
    PARSER_FULL_VERBOSE_ONLY(log("Current token: \"" + _next_token.value().get_value() + "\""););
    if (check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, ")"))
    {
        advance_token();
    }
    else
    {
        return false;
    }

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_dispatch_list"));

    return true;
}
// --------------------------------------- Parse arithmetic or logical operators ---------------------------------------
bool Parser::token_is_left_assoc_operator() const
{
    auto oper = _next_token.value().get_value();
    return oper == "<" || oper == "+" || oper == "-" || oper == "*" || oper == "/";
}

bool Parser::token_is_non_assoc_operator() const
{
    return _next_token.value().get_value() == "=" || _next_token.value().get_type() == lexer::Token::LE;
}

int Parser::precedence_level(const std::string &oper, const lexer::Token::TOKEN_TYPE &type) const
{
    if (type == lexer::Token::ASSIGN)
    {
        return 0;
    }
    else if (type == lexer::Token::NOT)
    {
        return 1;
    }
    else if (oper == "<")
    {
        return 2;
    }
    if (oper == "=" || type == lexer::Token::LE)
    {
        return 3;
    }
    if (oper == "+" || oper == "-")
    {
        return 4;
    }
    if (oper == "*" || oper == "/")
    {
        return 5;
    }
    else if (oper == "~")
    {
        return 6;
    }
    else if (type == lexer::Token::ISVOID)
    {
        return 7;
    }
#ifdef PARSER_FULL_VERBOSE
    assert(false && ("Parser::precedence_level: unexpected operator!"));
#endif //PARSER_FULL_VERBOSE
}

std::shared_ptr<ast::Expression> Parser::parse_operators(const std::shared_ptr<ast::Expression> &lhs)
{
    auto oper = _next_token.value().get_value();
    if (oper == "+")
    {
        return parse_operator<ast::PlusExpression>(lhs);
    }
    else if (oper == "-")
    {
        return parse_operator<ast::MinusExpression>(lhs);
    }
    else if (oper == "*")
    {
        return parse_operator<ast::MulExpression>(lhs);
    }
    else if (oper == "/")
    {
        return parse_operator<ast::DivExpression>(lhs);
    }
    else if (oper == "<")
    {
        return parse_operator<ast::LTExpression>(lhs);
    }
    else if (oper == "=")
    {
        return parse_operator<ast::EqExpression>(lhs);
    }
    else if (_next_token.value().get_type() == lexer::Token::LE)
    {
        return parse_operator<ast::LEExpression>(lhs);
    }

    return lhs;
}

// --------------------------------------- Methods for parsing expression with complex type of starting ---------------------------------------

std::shared_ptr<ast::Type> Parser::parse_type()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_type"));

    auto type = std::make_shared<ast::Type>();

    act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                    type->_string = _next_token.value().get_value());
    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_type"));
    return type;
}

std::shared_ptr<ast::ObjectExpression> Parser::parse_object()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_object"));

    auto obj = std::make_shared<ast::ObjectExpression>();

    act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                    obj->_object = _next_token.value().get_value());
    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_object"));
    return obj;
}

std::shared_ptr<ast::Expression> Parser::parse_expr_object()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_expr_object"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    auto lhs = parse_object();

    switch (_next_token.value().get_type())
    {
    case lexer::Token::ASSIGN:
    {
        advance_and_return_if_eof();

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_expr_object for ASSIGN"));
        return make_expr(ast::AssignExpression{lhs, parse_expr()}, PARSER_VERBOSE_LINE(line));
    }
    case lexer::Token::OPERATIONS_AND_CONTROLS:
    {
        if (_next_token.value().get_value() == "(")
        {
            ast::DispatchExpression dispatch;
            dispatch._expr = make_expr(ast::ObjectExpression{"self"}, PARSER_VERBOSE_LINE(line));
            dispatch._object = lhs;

            return_if_false(parse_dispatch_list(dispatch._args));

            PARSER_FULL_VERBOSE_ONLY(log_exit("parse_expr_object for ("));
            return make_expr(dispatch, PARSER_VERBOSE_LINE(line));
        }
    }
    }

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_expr_object"));
    return make_expr(*lhs, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_int()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_int"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    int value = std::stoi(_next_token.value().get_value());
    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_int"));
    return make_expr(ast::IntExpression{value}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_string()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_string"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    auto value = _next_token.value().get_value();
    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_string"));
    return make_expr(ast::StringExpression{value}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_bool()
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_bool"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    auto value = _next_token.value().get_value() == "true";
    advance_and_return_if_eof();

    PARSER_FULL_VERBOSE_ONLY(log_exit("parse_bool"));
    return make_expr(ast::BoolExpression{value}, PARSER_VERBOSE_LINE(line));
}

std::shared_ptr<ast::Expression> Parser::parse_maybe_dispatch_or_oper(
    const std::shared_ptr<ast::Expression> &expr)
{
    PARSER_FULL_VERBOSE_ONLY(log_enter("parse_maybe_dispatch_or_oper"));
    PARSER_VERBOSE_ONLY(int line = _next_token.value().get_line_number(););

    ast::DispatchExpression dispatch;
    dispatch._expr = expr;
    bool is_dispatch = false;

    auto oper = _next_token.value().get_value();
    if (oper == "@")
    {
        is_dispatch = true;
        dispatch._base = ast::StaticDispatchExpression();

        advance_and_return_if_eof();
        act_else_return(check_next_and_report_error(lexer::Token::TYPEID),
                        std::get<ast::StaticDispatchExpression>(dispatch._base)._type = parse_type());
    }
    else if (oper == ".")
    {

        is_dispatch = true;
        dispatch._base = ast::ObjectDispatchExpression();
    }

    if (is_dispatch)
    {
        _min_precedence = -1;

        advance_else_return(check_next_and_report_error(lexer::Token::OPERATIONS_AND_CONTROLS, "."));
        act_else_return(check_next_and_report_error(lexer::Token::OBJECTID),
                        dispatch._object = parse_object());

        return_if_false(parse_dispatch_list(dispatch._args));

        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_maybe_dispatch_or_oper for dispatch"));
        return parse_maybe_dispatch_or_oper(make_expr(dispatch, PARSER_VERBOSE_LINE(line)));
    }
    else
    {
        PARSER_FULL_VERBOSE_ONLY(log_exit("parse_maybe_dispatch_or_oper"));
        return parse_operators(expr);
    }
}

#ifdef PARSER_STANDALONE
int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        Parser p(std::make_shared<lexer::Lexer>(argv[i]));
        auto ast = p.parse_program();
        if (ast != nullptr)
        {
            ast::dump_program(ast);
        }
    }
}
#endif //PARSER_STANDALONE
