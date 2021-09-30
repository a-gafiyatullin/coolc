#include <iostream>
#include <string>
#include "utils/Utils.h"
#include <map>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>

#ifdef LEXER_FULL_VERBOSE
#include <cassert>
#endif // LEXER_FULL_VERBOSE

namespace lexer
{
    class Token
    {
    public:
        enum TOKEN_TYPE
        {
            /* keywords and operations */
            CLASS,
            ELSE,
            FI,
            IF,
            IN,
            INHERITS,
            LET,
            LOOP,
            POOL,
            THEN,
            WHILE,
            CASE,
            ESAC,
            OF,
            NOT,
            NEW,
            ISVOID,
            DARROW,
            ASSIGN,
            LE,
            /* objects */
            INT_CONST,
            BOOL_CONST,
            TYPEID,
            OBJECTID,
            STR_CONST,
            /* other */
            OPERATIONS_AND_CONTROLS,
            /* error */
            ERROR
        };

    protected:
        // name to token and token to name
        static const std::map<std::string, TOKEN_TYPE> _str_to_token_type;
        static const std::vector<std::string> _token_type_to_str;

        // list of control characters
        static const std::string _control_characters;

        TOKEN_TYPE _type;
        std::string _lexeme;

        int _line_number;

    public:
        Token(const TOKEN_TYPE &type, const std::string &substr, const int &line_number)
            : _type(type), _lexeme(substr), _line_number(line_number)
        {
        }

        inline std::string get_value() const { return _lexeme; }
        inline TOKEN_TYPE get_type() const { return _type; }

        inline static TOKEN_TYPE str_to_token(const std::string &str) { return _str_to_token_type.find(str)->second; }

        // try to determine token type by the first character in str
        inline static bool is_keyword(const std::string &str) { return !(_str_to_token_type.find(str) == _str_to_token_type.end()); }
        inline static bool is_whitespace(const std::string &str) { return (str[0] == ' ' || str[0] == '\f' || str[0] == '\r' || str[0] == '\t' || str[0] == '\v'); }
        inline static bool is_number(const std::string &str) { return isdigit(str[0]); }
        static bool is_boolean(const std::string &str);
        inline static bool is_typeid(const std::string &str) { return str[0] >= 'A' && str[0] <= 'Z' && !is_keyword(str); }
        inline static bool is_object(const std::string &str) { return str[0] >= 'a' && str[0] <= 'z' && !is_keyword(str); }
        // null character can be found in  _control_characters, although it is not a control character
        inline static bool is_control_character(const std::string &str) { return str[0] != 0 && _control_characters.find(str) != std::string::npos; }
        inline static bool is_close_par_comment(const std::string &str) { return str == "*)"; }

#ifdef LEXER_VERBOSE
        std::string to_string() const;

        inline void display() const
        {
            std::cout << to_string() << std::endl;
        }
#endif // LEXER_VERBOSE
    };
}