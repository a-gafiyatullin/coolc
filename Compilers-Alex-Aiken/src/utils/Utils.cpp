#include "utils/Utils.h"

#if defined(LEXER_VERBOSE) || defined(PARSER_VERBOSE)
#include <sstream>
#include <iomanip>

std::string get_printable_string(const std::string &str)
{
    // transform escape sequencies
    std::string str_for_out;
    for (int i = 0; i < str.size(); i++)
    {
        std::string ch = str.substr(i, 1);
        switch (str[i])
        {
        case '\n':
            ch = "\\n";
            break;
        case '\b':
            ch = "\\b";
            break;
        case '\t':
            ch = "\\t";
            break;
        case '\f':
            ch = "\\f";
            break;
        case '\\':
            ch = "\\\\";
            break;
        case '"':
            ch = "\\\"";
            break;
        default:
            if (!isprint(str[i]))
            {
                std::stringstream ss;
                ss << '\\' << std::oct << std::setfill('0') << std::setw(3)
                   << (int)str[i] << std::dec
                   << std::setfill(' ');
                ch = ss.str();
            }
        }
        str_for_out += ch;
    }
    str_for_out = "\"" + str_for_out + "\"";

    return str_for_out;
}
#endif // LEXER_VERBOSE || PARSER_VERBOSE