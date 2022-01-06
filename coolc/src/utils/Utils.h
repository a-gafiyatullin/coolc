#pragma once

#ifdef DEBUG
#include <iomanip>
#include <sstream>
#include <string>

#include <string.h>

extern bool TraceLexer;
extern bool PrintFinalAST;
extern bool TraceParser;
extern bool TraceSemant;
extern bool TraceCodeGen;

/**
 * @brief Process command line arguments
 *
 * @param args All command line arguments
 * @param args_num Number of command line arguments
 * @return Position of the first non-flag command line argument
 */
int process_args(const char *args[], const int &args_num);

/**
 * @brief Get the printable string object
 *
 * @param str original string
 * @return std::string after transformation
 */
std::string get_printable_string(const std::string &str);

#define DEBUG_ONLY(code) code
#define LEXER_VERBOSE_ONLY(text)                                                                                       \
    if (TraceLexer)                                                                                                    \
    {                                                                                                                  \
        text;                                                                                                          \
    }
#define PARSER_VERBOSE_ONLY(text)                                                                                      \
    if (TraceParser)                                                                                                   \
    {                                                                                                                  \
        text;                                                                                                          \
    }
#define SEMANT_VERBOSE_ONLY(text)                                                                                      \
    if (TraceSemant)                                                                                                   \
    {                                                                                                                  \
        text;                                                                                                          \
    }
#define CODEGEN_VERBOSE_ONLY(text)                                                                                     \
    if (TraceCodeGen)                                                                                                  \
    {                                                                                                                  \
        text;                                                                                                          \
    }

#else
#define DEBUG_ONLY(code)
#define LEXER_VERBOSE_ONLY(text)
#define PARSER_VERBOSE_ONLY(text)
#define SEMANT_VERBOSE_ONLY(text)
#define CODEGEN_VERBOSE_ONLY(text)
#endif // DEBUG