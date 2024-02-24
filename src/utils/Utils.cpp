#include "utils/Utils.h"
#include <unordered_map>

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

bool TraceLexer = false;
bool TokensOnly = false;
bool PrintFinalAST = false;
bool TraceParser = false;
bool TraceSemant = false;
bool TraceCodeGen = false;
bool TraceOpts = false;
bool VerifyOops = true;
#endif // DEBUG

#ifdef LLVM
bool UseArchSpecFeatures = true;
bool DoOpts = true;

#ifdef LLVM_SHADOW_STACK
bool ReduceGCSpills = true;
#endif // LLVM_SHADOW_STACK

#endif // LLVM

#ifdef MYIR
#ifdef DEBUG
bool PrintDominanceInfo = false;
bool TraceSSAConstruction = false;
#endif // DEBUG
#endif

#define flag_pair(flag)                                                                                                \
    {                                                                                                                  \
#flag, &flag                                                                                                   \
    }

std::unordered_map<std::string, bool *> BoolFlags = {
#ifdef DEBUG
    flag_pair(TraceLexer),
    flag_pair(TokensOnly),
    flag_pair(TraceLexer),
    flag_pair(PrintFinalAST),
    flag_pair(TraceParser),
    flag_pair(TraceSemant),
    flag_pair(TraceCodeGen),
    flag_pair(TraceOpts),
    flag_pair(VerifyOops)
#endif // DEBUG
#ifdef LLVM
#ifdef DEBUG
        ,
#endif // DEBUG
    flag_pair(UseArchSpecFeatures),
    flag_pair(DoOpts)
#ifdef LLVM_SHADOW_STACK
        ,
    flag_pair(ReduceGCSpills)
#endif // LLVM_SHADOW_STACK
#endif // LLVM

#ifdef MYIR
#ifdef DEBUG
        ,
    flag_pair(PrintDominanceInfo),
    flag_pair(TraceSSAConstruction)
#endif // DEBUG
#endif // MYIR
};

bool maybe_set(const char *arg)
{
    auto bool_flags = BoolFlags.find(arg + 1);
    if (bool_flags != BoolFlags.end())
    {
        *(bool_flags->second) = arg[0] == '+';
        return true;
    }

    return false;
}

std::pair<std::vector<int>, std::string> process_args(char *const args[], const int &args_num)
{
    std::string out_file_name;
    bool found_out_file_name = false;

    std::vector<int> positions;
    for (int i = 1; i < args_num; i++)
    {
        if (args[i][0] == '-' || args[i][0] == '+')
        {
            if (maybe_set(args[i]))
            {
                continue;
            }

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
