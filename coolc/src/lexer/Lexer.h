#pragma once

#include "Token.h"
#include <fstream>
#include <optional>
#include <queue>
#include <regex>

#ifdef DEBUG
#include "utils/logger/Logger.h"
#endif // DEBUG

namespace lexer
{

/**
 * @brief Build tokens stream from input file
 *
 */
class Lexer
{
  private:
    static constexpr int MAX_STR_CONST = 1025;

    std::ifstream _input_file;
    const std::string _file_name;
    int _line_number;

    // lexer specification that is used outside of Cool strings and Cool comments
    static const std::regex LEXER_SPEC_REGEX;

    static const std::regex STR_SYMBOLS_REGEX;
    static const std::regex COMM_SYMBOLS_REGEX;

    std::string _current_line;
    std::queue<Token> _saved_tokens; // see next() description

    // return either STR_CONST or ERROR token
    Token match_string(const std::string &start_string);
    // return either std::nullopt if OK or ERROR token
    std::optional<Token> skip_comment(const std::string &start_string);

    void append_to_string_if_can(std::string &prefix, const std::string &suffix, std::string &error_msg,
                                 int &error_line_num);

#ifdef DEBUG
    void log_match(const std::string &type, const std::string &str, const int &pos);
#endif // DEBUG

  public:
    /**
     * @brief Construct a new Lexer object
     *
     * @param input_file_name Name of the source file
     */
    Lexer(const std::string &input_file_name);

    /**
     * @brief Get maybe next token
     *
     * @return Next token or nullopt
     *
     * @details
     * next() reads a line from the _input_file and save it in the _current_line.
     * next() tries to match all possible tokens in the _current_line during a call and save them in the
     * _saved_tokens. next next() calls will be pop tokens from the _saved_tokens if it is not empty. next() can read
     * more than one line if it recognised start of Cool string or Cool comment, in such cases next() will read new
     * lines until read EOF, end of string or end of comment. next() returns std::nullopt for EOF.
     */
    std::optional<Token> next();

    /**
     * @brief Get the file name
     *
     * @return File name string
     */
    inline std::string get_file_name() const
    {
        return _file_name;
    }

    /**
     * @brief Get the line number
     *
     * @return Current line number in file
     */
    inline int get_line_number() const
    {
        return _line_number;
    }
};

} // namespace lexer