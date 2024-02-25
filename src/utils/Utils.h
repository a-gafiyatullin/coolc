#pragma once

#include <string>
#include <vector>

#ifdef LLVM
extern bool UseArchSpecFeatures;
extern bool DoOpts;

#ifdef LLVM_SHADOW_STACK
extern bool ReduceGCSpills;
#endif // LLVM_SHADOW_STACK

#endif // LLVM

/**
 * @brief Process command line arguments
 *
 * @param args All command line arguments
 * @param args_num Number of command line arguments
 * @return Positions of the all non-flag command line argument and output file name
 */
std::pair<std::vector<int>, std::string> process_args(char *const args[], const int &args_num);

#ifdef DEBUG
#include <cassert>

extern bool TraceLexer;
extern bool TokensOnly;
extern bool PrintFinalAST;
extern bool TraceParser;
extern bool TraceSemant;
extern bool TraceCodeGen;
extern bool TraceOpts;
extern bool VerifyOops;

#ifdef MYIR
extern bool PrintDominanceInfo;
extern bool TraceSSAConstruction;
#endif // MYIR

/**
 * @brief Get the printable string object
 *
 * @param str Original string
 * @return String after transformation
 */
std::string printable_string(const std::string &str);

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
#define OPT_VERBOSE_ONLY(text)                                                                                         \
    if (TraceOpts)                                                                                                     \
    {                                                                                                                  \
        text;                                                                                                          \
    }

#define GUARANTEE_DEBUG(expr) assert(expr)
#define SHOULD_NOT_REACH_HERE() assert(false && "Should not reach here!")

#else

#define DEBUG_ONLY(code)
#define LEXER_VERBOSE_ONLY(text)
#define PARSER_VERBOSE_ONLY(text)
#define SEMANT_VERBOSE_ONLY(text)
#define CODEGEN_VERBOSE_ONLY(text)
#define OPT_VERBOSE_ONLY(text)

#define GUARANTEE_DEBUG(expr)
#define SHOULD_NOT_REACH_HERE()

#endif // DEBUG
