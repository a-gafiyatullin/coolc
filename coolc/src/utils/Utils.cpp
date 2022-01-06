#include "utils/Utils.h"

#ifdef DEBUG
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
                ss << '\\' << std::oct << std::setfill('0') << std::setw(3) << (int)str[i] << std::dec
                   << std::setfill(' ');
                ch = ss.str();
            }
        }
        str_for_out += ch;
    }
    str_for_out = "\"" + str_for_out + "\"";

    return str_for_out;
}

bool TraceLexer;
bool PrintFinalAST;
bool TraceParser;
bool TraceSemant;
bool TraceCodeGen;

bool maybe_set(const char *arg, const char *flag_name, bool &flag)
{
    if (!strcmp(flag_name, arg + 1))
    {
        if (arg[0] == '+')
        {
            flag = true;
            return true;
        }
    }

    return false;
}

int process_args(const char *args[], const int &args_num)
{
    TraceLexer = false;
    PrintFinalAST = false;
    TraceParser = false;
    TraceSemant = false;
    TraceCodeGen = false;

    int i = 0;
    for (; i < args_num; i++)
    {
        if (args[i][0] == '-' || args[i][0] == '+')
        {
            if (maybe_set(args[i], "TraceLexer", TraceLexer))
            {
                continue;
            }
            if (maybe_set(args[i], "PrintFinalAST", PrintFinalAST))
            {
                continue;
            }
            if (maybe_set(args[i], "TraceParser", TraceParser))
            {
                continue;
            }
            if (maybe_set(args[i], "TraceSemant", TraceSemant))
            {
                continue;
            }
            if (maybe_set(args[i], "TraceCodeGen", TraceCodeGen))
            {
                continue;
            }
            break;
        }
    }

    return i;
}
#endif // DEBUG