#include "lexer/Token.h"

using namespace lexer;

const std::vector<std::string> Token::_token_type_to_str = {
    "CLASS",
    "ELSE",
    "FI",
    "IF",
    "IN",
    "INHERITS",
    "LET",
    "LOOP",
    "POOL",
    "THEN",
    "WHILE",
    "CASE",
    "ESAC",
    "OF",
    "NOT",
    "NEW",
    "ISVOID",
    "DARROW",
    "ASSIGN",
    "LE",
    /* objects */
    "INT_CONST",
    "BOOL_CONST",
    "TYPEID",
    "OBJECTID",
    "STR_CONST",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "ERROR"};

#ifdef LEXER_VERBOSE
std::string Token::to_string() const
{
    std::string out = "#" + std::to_string(_line_number) + " ";
    if (_type >= SEMICOLON && _type <= COMMA)
    {
        out += "\'" + _lexeme + "\'";
    }
    else
    {
        out += _token_type_to_str[_type];
        if (_type >= INT_CONST && _type < STR_CONST)
        {
            out += " " + _lexeme;
        }
        else if (_type == STR_CONST || _type == ERROR)
        {
            out += " " + get_printable_string(_lexeme);
        }
    }

    return out;
}
#endif // LEXER_VERBOSE

const std::unordered_map<std::string, Token::TOKEN_TYPE> Token::_str_to_token_type = {
    {"class", Token::TOKEN_TYPE::CLASS},
    {"else", Token::TOKEN_TYPE::ELSE},
    {"fi", Token::TOKEN_TYPE::FI},
    {"if", Token::TOKEN_TYPE::IF},
    {"in", Token::TOKEN_TYPE::IN},
    {"inherits", Token::TOKEN_TYPE::INHERITS},
    {"let", Token::TOKEN_TYPE::LET},
    {"loop", Token::TOKEN_TYPE::LOOP},
    {"pool", Token::TOKEN_TYPE::POOL},
    {"then", Token::TOKEN_TYPE::THEN},
    {"while", Token::TOKEN_TYPE::WHILE},
    {"case", Token::TOKEN_TYPE::CASE},
    {"esac", Token::TOKEN_TYPE::ESAC},
    {"of", Token::TOKEN_TYPE::OF},
    {"not", Token::TOKEN_TYPE::NOT},
    {"new", Token::TOKEN_TYPE::NEW},
    {"isvoid", Token::TOKEN_TYPE::ISVOID},
    {";", Token::TOKEN_TYPE::SEMICOLON},
    {"{", Token::TOKEN_TYPE::LEFT_CURLY_BRACKET},
    {"}", Token::TOKEN_TYPE::RIGHT_CURLY_BRACKET},
    {":", Token::TOKEN_TYPE::COLON},
    {"(", Token::TOKEN_TYPE::LEFT_PAREN},
    {")", Token::TOKEN_TYPE::RIGHT_PAREN},
    {".", Token::TOKEN_TYPE::DOT},
    {"@", Token::TOKEN_TYPE::AT},
    {"~", Token::TOKEN_TYPE::NEG},
    {"*", Token::TOKEN_TYPE::ASTERISK},
    {"/", Token::TOKEN_TYPE::SLASH},
    {"+", Token::TOKEN_TYPE::PLUS},
    {"-", Token::TOKEN_TYPE::MINUS},
    {"<", Token::TOKEN_TYPE::LESS},
    {"=", Token::TOKEN_TYPE::EQUALS},
    {",", Token::TOKEN_TYPE::COMMA},
    {"=>", Token::TOKEN_TYPE::DARROW},
    {"<-", Token::TOKEN_TYPE::ASSIGN},
    {"<=", Token::TOKEN_TYPE::LE}};

bool Token::is_boolean(const std::string &str)
{
    std::string str_in_lowercase = str;
    if (str[0] != 'f' && str[0] != 't')
    {
        return false;
    }
    std::transform(str_in_lowercase.begin(), str_in_lowercase.end(), str_in_lowercase.begin(), ::tolower);

    return (str_in_lowercase == "true" || str_in_lowercase == "false");
}