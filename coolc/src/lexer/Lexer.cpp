#include "lexer/Lexer.h"

using namespace lexer;

const std::regex Lexer::LEXER_SPEC_REGEX(
    // keywords
    "[cC][lL][aA][sS][sS]|[eE][lL][sS][eE]|[fF][iI]|[iI][fF]"
    "|[iI][nN]|[iI][nN][hH][eE][rR][iI][tT][sS]|[lL][eE][tT]|"
    "[lL][oO][oO][pP]|[pP][oO][oO][lL]|[tT][hH][eE][nN]|[wW]"
    "[hH][iI][lL][eE]|[cC][aA][sS][eE]|[eE][sS][aA][cC]|[oO][fF]|"
    "[nN][oO][tT]|[nN][eE][wW]|[iI][sS][vV][oO][iI][dD]|"
    "=>|<=|<-|"
    // types and objects
    "[0-9]+|t[rR][uU][eE]|f[aA][lL][sS][eE]|[A-Z][a-zA-Z0-9_]*|[a-z][a-zA-Z0-9_]*|"
    // control symbols
    ";|\\{|}|:|\\(|\\)|\\."
    "@|~|\\*|/|\\+|-|<|=|,|"
    "\\*\\)|"
    // whitespaces
    "[\f\r\t\v ]+|"
    // error
    "." // . does not accept \x00
#ifndef __APPLE__
    "|\x00"
#endif // NOT __APPLE__
    ,
    std::regex::extended);

const std::regex Lexer::STR_SYMBOLS_REGEX("\\\"|\\\\|\\x00");
const std::regex Lexer::COMM_SYMBOLS_REGEX("\\(\\*|\\*\\)");

Lexer::Lexer(const std::string &input_file_name) : _file_name(input_file_name)
{
    _line_number = 0;

    _input_file.open(input_file_name);
    if (!_input_file.is_open())
    {
        throw std::runtime_error("Lexer::Lexer: can't open file " + input_file_name + "!");
    }
}

// append a suffix to prefix if can
// set new error message
void Lexer::append_to_string_if_can(std::string &prefix, const std::string &suffix, std::string &error_msg,
                                    int &error_line_num)
{
    if (!error_msg.empty())
    {
        return;
    }
    if (prefix.length() + suffix.length() > MAX_STR_CONST - 1)
    {
        error_msg = "String constant too long";
        error_line_num = _line_number;
    }
    else
    {
        prefix += suffix;
    }
}

Token Lexer::match_string(const std::string &start_string)
{

    bool escape = false;
    std::string error_msg;
    int error_line_num;

    std::string processed_string = start_string;
    std::string builded_string;

    while (true)
    {
        // we read all lines from the file
        if (_input_file.eof() && processed_string.empty())
        {
            if (error_msg.empty())
            {
                error_msg = "EOF in string constant";
                error_line_num = _line_number;
            }

            _current_line.clear();
            return Token(Token::ERROR, error_msg, error_line_num);
        }
        if (processed_string.empty())
        {
            std::getline(_input_file, processed_string);
            _line_number++;

            LEXER_VERBOSE_ONLY(LOG("New line: " + processed_string));
            LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("new line symbol", "", 0)));

            // we read a new line, so check if '\n' is escaped
            if (!escape)
            {
                // save a rest of the line
                _current_line = processed_string;
                return Token(Token::ERROR, error_msg.empty() ? "Unterminated string constant" : error_msg,
                             error_msg.empty() ? _line_number : error_line_num);
            }

            append_to_string_if_can(builded_string, "\n", error_msg, error_line_num);
            escape = 0;
        }

        // escape next character
        if (escape)
        {
            // we analyze the first character not in the regular expression,
            // so catch the special case when it is \x00
            if (processed_string[0] == '\0')
            {
                if (error_msg.empty())
                {
                    error_msg = "String contains escaped null character.";
                    error_line_num = _line_number;
                }
            }
            else
            {
                std::string ch = processed_string.substr(0, 1);
                switch (processed_string[0])
                {
                case 'n':
                    ch = "\n";
                    break;
                case 'b':
                    ch = "\b";
                    break;
                case 't':
                    ch = "\t";
                    break;
                case 'f':
                    ch = "\f";
                    break;
                case '\\':
                    ch = "\\";
                }

                append_to_string_if_can(builded_string, ch, error_msg, error_line_num);
            }
            escape = 0;
            processed_string = processed_string.substr(1);
        }

        // match escape characters and closing "
        std::smatch matches;
        if (std::regex_search(processed_string, matches, STR_SYMBOLS_REGEX))
        {
            int ch = processed_string[matches.position(0)];

            append_to_string_if_can(builded_string, processed_string.substr(0, matches.position(0)), error_msg,
                                    error_line_num);
            processed_string = processed_string.substr(matches.position(0) + 1);

            switch (ch)
            {
            case '\"': {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("closing \"", "", matches.position(0))));

                _current_line = processed_string;
                return Token(error_msg.empty() ? Token::STR_CONST : Token::ERROR,
                             error_msg.empty() ? builded_string : error_msg,
                             error_msg.empty() ? _line_number : error_line_num);
            }
            case '\\': {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("closing \\", "", matches.position(0))));

                if (error_msg.empty())
                {
                    escape = true;
                }
                break;
            }
            case '\0': {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("null character", "", matches.position(0))));

                if (error_msg.empty())
                {
                    error_msg = "String contains null character.";
                    error_line_num = _line_number;
                }
                break;
            }
            }
        }
        else
        {
            // did not catch any special characters using regular expression
            append_to_string_if_can(builded_string, processed_string, error_msg, error_line_num);
            processed_string.clear();
        }
    }
}

std::optional<Token> Lexer::skip_comment(const std::string &start_string)
{
    std::string processed_string = start_string;
    int comm_level = 1;

    while (true)
    {
        if (_input_file.eof() && processed_string.empty())
        {
            _current_line.clear();
            return Token(Token::ERROR, "EOF in comment", _line_number);
        }
        if (processed_string.empty())
        {
            std::getline(_input_file, processed_string);
            _line_number++;

            LEXER_VERBOSE_ONLY(LOG("New line: " + processed_string));
        }

        std::smatch matches;
        if (std::regex_search(processed_string, matches, COMM_SYMBOLS_REGEX))
        {
            for (int i = 0; i < matches.size(); i++)
            {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("controls", matches[i], matches.position(i))));

                char ch = processed_string[matches.position(i)];
                if (ch == '(')
                {
                    comm_level++; // open a nested comment
                }
                else
                {
                    comm_level--; // close a nested comment
                }
                // all comments are closed
                if (comm_level == 0)
                {
                    _current_line = processed_string.substr(matches.position(i) + 2);
                    return std::nullopt;
                }
            }
            processed_string = processed_string.substr(matches.position(matches.size() - 1) + 2);
        }
        else
        {
            // skip this line
            processed_string.clear();
        }
    }
}

std::optional<Token> Lexer::next()
{
    std::string current_line_suffix;
    if (_saved_tokens.empty())
    {
        while (_saved_tokens.empty())
        {
            if (_current_line.empty())
            {
                std::getline(_input_file, _current_line);
                _line_number++;

                LEXER_VERBOSE_ONLY(LOG("New line: " + _current_line));
            }
            if (_input_file.eof() && _current_line.empty())
            {
                // end of file
                return std::nullopt;
            }

            // try to match Cool string
            auto string_start = _current_line.find("\"");
            // try to match Cool comments
            auto par_comm_start = _current_line.find("(*");
            auto dash_comm_start = _current_line.find("--");

            int suffix_start = -1;
            int shift = -1;
            // save string suffix for further lexing
            if (string_start < par_comm_start && string_start < dash_comm_start)
            {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("start of the string", "", string_start)));
                suffix_start = string_start;
                shift = 1;
            }
            else if (par_comm_start < string_start && par_comm_start < dash_comm_start)
            {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("start of the comment", "", par_comm_start)));
                suffix_start = par_comm_start;
                shift = 2;
            }
            else if (dash_comm_start < string_start && dash_comm_start < par_comm_start)
            {
                LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("start of the dash comment", "", dash_comm_start)));
                suffix_start = dash_comm_start;
                shift = 2;
            }
            if (suffix_start != -1)
            {
                current_line_suffix = _current_line.substr(suffix_start + shift);
                _current_line = _current_line.substr(0, suffix_start);
            }
            // throw out suffix for comments starting from --
            if (suffix_start == dash_comm_start)
            {
                current_line_suffix.clear();
                suffix_start = -1;
            }

            // match all substring before Cool string and Cool comments
            auto match_begin = std::sregex_iterator(_current_line.begin(), _current_line.end(), LEXER_SPEC_REGEX);
            auto match_end = std::sregex_iterator();

            for (auto it = match_begin; it != match_end; ++it)
            {
                std::string str_in_lowercase = it->str();
                std::transform(str_in_lowercase.begin(), str_in_lowercase.end(), str_in_lowercase.begin(), ::tolower);

#ifndef __APPLE__
                // we are not interested in characters (\x00) over the line length
                if (it->position() >= _current_line.size())
                {
                    continue;
                }
                // if we matched \x00 in the string so create a string containing \x00
                Token t(Token::ERROR, (it->str()).length() != 0 ? it->str() : std::string("\0", 1), _line_number);
#else
                Token t(Token::ERROR, it->str(), _line_number);
#endif // NOT __APPLE__
                if (Token::is_keyword(str_in_lowercase))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("keyword or symbol", it->str(), it->position())));
                    t = Token(Token::str_to_token(str_in_lowercase), it->str(), _line_number);
                }
                else if (Token::is_number(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("number", it->str(), it->position())));
                    t = Token(Token::INT_CONST, str_in_lowercase, _line_number);
                }
                else if (Token::is_boolean(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("boolean", it->str(), it->position())));
                    t = Token(Token::BOOL_CONST, str_in_lowercase, _line_number);
                }
                else if (Token::is_typeid(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("typeid", it->str(), it->position())));
                    t = Token(Token::TYPEID, it->str(), _line_number);
                }
                else if (Token::is_object(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("object", it->str(), it->position())));
                    t = Token(Token::OBJECTID, it->str(), _line_number);
                }
                else if (Token::is_close_par_comment(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("", it->str(), it->position())));
                    t = Token(Token::ERROR, "Unmatched *)", _line_number);
                }
                else if (Token::is_whitespace(it->str()))
                {
                    LEXER_VERBOSE_ONLY(LOG(LEXER_LOG_MATCH("whitespace", it->str(), it->position())));
                    continue;
                }
                _saved_tokens.push(t);
            }
            _current_line.clear();

            LEXER_VERBOSE_ONLY(LOG("Start analyzing a rest of the string."));

            // analyze a rest of the string
            if (suffix_start != -1)
            {
                // it is start of the Cool string
                if (suffix_start == string_start)
                {
                    _saved_tokens.push(match_string(current_line_suffix));
                }
                else
                {
                    // it is start of the Cool comment
                    auto t = skip_comment(current_line_suffix);
                    if (t.has_value())
                    {
                        _saved_tokens.push(t.value());
                    }
                }
            }

            LEXER_VERBOSE_ONLY(LOG("End analyzing the rest of the string."));
        }

        auto t = _saved_tokens.front();
        _saved_tokens.pop();

        DEBUG_ONLY(if (TokensOnly) { LOG(t.to_string()); });

        return t;
    }

    auto t = _saved_tokens.front();
    _saved_tokens.pop();

    DEBUG_ONLY(if (TokensOnly) { LOG(t.to_string()); });

    return t;
}