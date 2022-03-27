#pragma once

#include "utils/Utils.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace lexer
{

class Token
{
  public:
    enum TokenType
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
        SEMICOLON,
        LEFT_CURLY_BRACKET,
        RIGHT_CURLY_BRACKET,
        COLON,
        LEFT_PAREN,
        RIGHT_PAREN,
        DOT,
        AT,
        NEG,
        ASTERISK,
        SLASH,
        PLUS,
        MINUS,
        LESS,
        EQUALS,
        COMMA,
        /* error */
        ERROR
    };

  private:
    // name to token and token to name
    static const std::unordered_map<std::string, TokenType> STR_TO_TOKEN_TYPE;
    static const std::vector<std::string> TOKEN_TYPE_TO_STR;

    TokenType _type;
    std::string _lexeme;

    int _line_number;

  public:
    /**
     * @brief Construct a new Token
     *
     * @param type Type of token from TOKEN_TYPE
     * @param substr Lexeme
     * @param line_number Where this token was found in the file
     */
    Token(const TokenType &type, const std::string &substr, const int &line_number)
        : _type(type), _lexeme(substr), _line_number(line_number)
    {
    }

    /**
     * @brief Get token type for Cool reserved words
     *
     * @param str Lexeme
     * @return Token type for lexeme
     */
    inline static TokenType str_to_token(const std::string &str)
    {
        return STR_TO_TOKEN_TYPE.find(str)->second;
    }

    /**
     * @brief Check if lexeme is a keyword
     *
     * @param str Lexeme
     * @return True if it is a keyword
     */
    inline static bool is_keyword(const std::string &str)
    {
        return !(STR_TO_TOKEN_TYPE.find(str) == STR_TO_TOKEN_TYPE.end());
    }

    /**
     * @brief Check if lexeme is a whitespace
     *
     * @param str Lexeme
     * @return True if it is a whitespace
     */
    inline static bool is_whitespace(const std::string &str)
    {
        return (str[0] == ' ' || str[0] == '\f' || str[0] == '\r' || str[0] == '\t' || str[0] == '\v');
    }

    /**
     * @brief Check if lexeme is a number
     *
     * @param str Lexeme
     * @return True if it is a number
     */
    inline static bool is_number(const std::string &str)
    {
        return isdigit(str[0]);
    }

    /**
     * @brief Check if lexeme is a boolean
     *
     * @param str Lexeme
     * @return True if it is a boolean
     */
    static bool is_boolean(const std::string &str);

    /**
     * @brief Check if lexeme is a type name
     *
     * @param str Lexeme
     * @return True if it is a type name
     */
    inline static bool is_typeid(const std::string &str)
    {
        return str[0] >= 'A' && str[0] <= 'Z' && !is_keyword(str);
    }

    /**
     * @brief Check if lexeme is an object name
     *
     * @param str Lexeme
     * @return True if it is an object name
     */
    inline static bool is_object(const std::string &str)
    {
        return str[0] >= 'a' && str[0] <= 'z' && !is_keyword(str);
    }

    /**
     * @brief Check if lexeme is a closing comment bracket
     *
     * @param str Lexeme
     * @return True if it a closing comment bracket
     */
    inline static bool is_close_par_comment(const std::string &str)
    {
        return str == "*)";
    }

    /**
     * @brief Get the type as string
     *
     * @return String for type
     */
    inline std::string type_as_str() const
    {
        return TOKEN_TYPE_TO_STR[_type];
    }

    /**
     * @brief Get the line number
     *
     * @return Line number
     */
    inline int line_number() const
    {
        return _line_number;
    }

    /**
     * @brief Check if this token has given type
     *
     * @param type Type from TOKEN_TYPE
     * @return True if this token has given type
     */
    inline bool same_token_type(const TokenType &type) const
    {
        return _type == type;
    }

    /**
     * @brief Get the lexeme
     *
     * @return Lexeme
     */
    inline std::string value() const
    {
        return _lexeme;
    }

    /**
     * @brief Get the type
     *
     * @return Type
     */
    inline TokenType type() const
    {
        return _type;
    }

#ifdef DEBUG
    /**
     * @brief Get string for logger
     *
     * @return String for debug
     */
    std::string to_string() const;
#endif // DEBUG
};

} // namespace lexer