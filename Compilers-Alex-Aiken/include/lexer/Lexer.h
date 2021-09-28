#include "Token.h"
#include <fstream>
#include <regex>
#include <optional>
#include <queue>

class Lexer
{
protected:
    static const int MAX_STR_CONST;

    std::ifstream _input_file;
    std::string _file_name;
    int _line_number;

    static const std::regex _lexer_spec; // lexer specification that is used outside of Cool strings and Cool comments

    static const std::regex _str_symbols_regexs;
    static const std::regex _comm_symbols_regexs;

    std::string _current_line;
    std::queue<Token> _saved_tokens; // see next() description

    // return either STR_CONST or ERROR token
    Token match_string(const std::string &start_string);
    // return either std::nullopt if OK or ERROR token
    std::optional<Token> skip_comment(const std::string &start_string);

    void append_to_string_if_can(std::string &prefix, const std::string &suffix,
                                 std::string &error_msg, int &error_line_num);

public:
    Lexer(const std::string &input_file_name);

    /* 
     * next() reads a line from the _input_file and save it in the _current_line.
     * next() tries to match all possible tokens in the _current_line during a call and save them in the _saved_tokens. 
     * next next() calls will be pop tokens from the _saved_tokens if it is not empty.
     * next() can read more than one line if it recognised start of Cool string or Cool comment,
     * in such cases next() will read new lines until read EOF, end of string or end of comment.
     * next() returns std::nullopt for EOF.
     */
    std::optional<Token> next();

#ifdef LEXER_VERBOSE
    std::string to_string() const;

    inline void display() const
    {
        std::cout << to_string() << std::endl;
    }
#endif // LEXER_VERBOSE
};