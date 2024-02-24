#include "Token.h"
#include "utils/Utils.h"

using namespace lexer;

const std::vector<std::string> Token::TOKEN_TYPE_TO_STR = {
    "CLASS", "ELSE", "FI", "IF", "IN", "INHERITS", "LET", "LOOP", "POOL", "THEN", "WHILE", "CASE", "ESAC", "OF", "NOT",
    "NEW", "ISVOID", "DARROW", "ASSIGN", "LE",
    /* objects */
    "INT_CONST", "BOOL_CONST", "TYPEID", "OBJECTID", "STR_CONST", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "ERROR"};

#ifdef DEBUG
std::string Token::to_string() const
{
    auto out = "#" + std::to_string(_line_number) + " ";
    if (_type >= SEMICOLON && _type <= COMMA)
    {
        out += "\'" + _lexeme + "\'";
    }
    else
    {
        out += TOKEN_TYPE_TO_STR[_type];
        if (_type >= INT_CONST && _type < STR_CONST)
        {
            out += " " + _lexeme;
        }
        else if (_type == STR_CONST || _type == ERROR)
        {
            out += " " + printable_string(_lexeme);
        }
    }

    return out;
}
#endif // DEBUG

const std::unordered_map<std::string, Token::TokenType> Token::STR_TO_TOKEN_TYPE = {
    {"class", Token::TokenType::CLASS},
    {"else", Token::TokenType::ELSE},
    {"fi", Token::TokenType::FI},
    {"if", Token::TokenType::IF},
    {"in", Token::TokenType::IN},
    {"inherits", Token::TokenType::INHERITS},
    {"let", Token::TokenType::LET},
    {"loop", Token::TokenType::LOOP},
    {"pool", Token::TokenType::POOL},
    {"then", Token::TokenType::THEN},
    {"while", Token::TokenType::WHILE},
    {"case", Token::TokenType::CASE},
    {"esac", Token::TokenType::ESAC},
    {"of", Token::TokenType::OF},
    {"not", Token::TokenType::NOT},
    {"new", Token::TokenType::NEW},
    {"isvoid", Token::TokenType::ISVOID},
    {";", Token::TokenType::SEMICOLON},
    {"{", Token::TokenType::LEFT_CURLY_BRACKET},
    {"}", Token::TokenType::RIGHT_CURLY_BRACKET},
    {":", Token::TokenType::COLON},
    {"(", Token::TokenType::LEFT_PAREN},
    {")", Token::TokenType::RIGHT_PAREN},
    {".", Token::TokenType::DOT},
    {"@", Token::TokenType::AT},
    {"~", Token::TokenType::NEG},
    {"*", Token::TokenType::ASTERISK},
    {"/", Token::TokenType::SLASH},
    {"+", Token::TokenType::PLUS},
    {"-", Token::TokenType::MINUS},
    {"<", Token::TokenType::LESS},
    {"=", Token::TokenType::EQUALS},
    {",", Token::TokenType::COMMA},
    {"=>", Token::TokenType::DARROW},
    {"<-", Token::TokenType::ASSIGN},
    {"<=", Token::TokenType::LE}};

bool Token::is_boolean(const std::string &str)
{
    auto str_in_lowercase = str;
    if (str[0] != 'f' && str[0] != 't')
    {
        return false;
    }
    std::transform(str_in_lowercase.begin(), str_in_lowercase.end(), str_in_lowercase.begin(), ::tolower);

    return (str_in_lowercase == "true" || str_in_lowercase == "false");
}
