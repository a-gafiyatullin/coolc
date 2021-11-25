#include "utils/logger/Logger.h"

#if defined(LEXER_FULL_VERBOSE) || defined(SEMANT_FULL_VERBOSE) || defined(CODEGEN_FULL_VERBOSE)
void Logger::update_ident()
{
    if (_parent_logger)
    {
        _ident = std::max(_ident, _parent_logger->_ident);
    }
}

void Logger::log(const std::string &msg)
{
    update_ident();
    std::cout << std::string(_ident, ' ') << msg << std::endl;
}

void Logger::log_enter(const std::string &msg)
{
    update_ident();

    _ident += _IDENT_SIZE;
    log("Enter " + msg);
}

void Logger::log_exit(const std::string &msg)
{
    update_ident();

    log("Exit " + msg);
    _ident -= _IDENT_SIZE;
}
#endif // LEXER_FULL_VERBOSE || SEMANT_FULL_VERBOSE || CODEGEN_FULL_VERBOSE