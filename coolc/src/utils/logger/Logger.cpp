#include "utils/logger/Logger.h"

#if defined(LEXER_FULL_VERBOSE) || defined(SEMANT_FULL_VERBOSE) || defined(CODEGEN_FULL_VERBOSE)
int Logger::update_ident()
{
    if (_parent_logger)
    {
        _ident = _parent_logger->update_ident();
    }

    return _ident;
}

void Logger::log(const std::string &msg)
{
    update_ident();
    std::cout << std::string(_ident, ' ') << msg << std::endl;
}

void Logger::log_enter(const std::string &msg)
{
    if (!_parent_logger)
    {
        _ident += _IDENT_SIZE;
    }
    log("Enter " + msg);
}

void Logger::log_exit(const std::string &msg)
{
    log("Exit " + msg);
    if (!_parent_logger)
    {
        _ident -= _IDENT_SIZE;
    }
}
#endif // LEXER_FULL_VERBOSE || SEMANT_FULL_VERBOSE || CODEGEN_FULL_VERBOSE