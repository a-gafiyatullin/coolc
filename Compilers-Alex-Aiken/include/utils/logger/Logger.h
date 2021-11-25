#pragma once

#if defined(LEXER_FULL_VERBOSE) || defined(SEMANT_FULL_VERBOSE) || defined(CODEGEN_FULL_VERBOSE)
#include <iostream>

class Logger
{
private:
    static constexpr int _IDENT_SIZE = 4;

    int _ident;
    const Logger *_parent_logger;

    void update_ident();

public:
    Logger() : _ident(0), _parent_logger(nullptr) {}
    Logger(const Logger *logger) : _ident(0), _parent_logger(logger) {}

    void log(const std::string &msg);
    void log_enter(const std::string &msg);
    void log_exit(const std::string &msg);
};
#endif // LEXER_FULL_VERBOSE || SEMANT_FULL_VERBOSE || CODEGEN_FULL_VERBOSE