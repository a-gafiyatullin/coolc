#include "utils/Utils.h"

#ifdef DEBUG
std::string printable_string(const std::string &str)
{
    // transform escape sequencies
    std::string str_for_out;
    for (auto i = 0; i < str.size(); i++)
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
bool TokensOnly;
bool PrintFinalAST;
bool TraceParser;
bool TraceSemant;
bool TraceCodeGen;
#endif // DEBUG

bool UseArchSpecFeatures;

bool maybe_set(const char *arg, const char *flag_name, bool &flag)
{
    if (!strcmp(flag_name, arg + 1))
    {
        if (arg[0] == '+')
        {
            flag = true;
        }
        return true;
    }

    return false;
}

#define check_flag(flag)                                                                                               \
    if (maybe_set(args[i], #flag, flag))                                                                               \
    {                                                                                                                  \
        continue;                                                                                                      \
    }

std::pair<std::vector<int>, std::string> process_args(char *const args[], const int &args_num)
{
#ifdef DEBUG
    TraceLexer = false;
    PrintFinalAST = false;
    TraceParser = false;
    TraceSemant = false;
    TraceCodeGen = false;
    TokensOnly = false;
#endif // DEBUG
    UseArchSpecFeatures = true;

    std::string out_file_name;
    bool found_out_file_name = false;

    std::vector<int> positions;
    for (int i = 1; i < args_num; i++)
    {
        if (args[i][0] == '-' || args[i][0] == '+')
        {
#ifdef DEBUG
            check_flag(TraceLexer);
            check_flag(PrintFinalAST);
            check_flag(TraceParser);
            check_flag(TraceSemant);
            check_flag(TokensOnly);
            check_flag(TraceCodeGen);
#endif // DEBUG
            check_flag(UseArchSpecFeatures);

            // output file name
            if (!strcmp(args[i], "-o"))
            {
                if (i + 1 < args_num)
                {
                    found_out_file_name = true;
                    out_file_name = args[++i];
                }
            }
        }
        else
        {
            positions.push_back(i);
        }
    }

    if (!found_out_file_name)
    {
        out_file_name = args[positions[0]];
        out_file_name = out_file_name.substr(0, out_file_name.find_last_of("."));
    }

    return {positions, out_file_name};
}