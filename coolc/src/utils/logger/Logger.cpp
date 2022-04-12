#include "utils/logger/Logger.h"

#ifdef DEBUG
std::shared_ptr<Logger> Logger::LOGGER = nullptr;

void Logger::log(const std::string &msg, const bool &cr, const bool &do_ident)
{
    if (do_ident)
    {
        std::cout << std::string(_ident, ' ');
    }

    std::cout << msg;

    if (cr)
    {
        std::cout << std::endl;
    }
}

void Logger::log_enter(const std::string &msg)
{
    log("ENTER " + msg);
    _ident += IDENT_SIZE;
}

void Logger::log_exit(const std::string &msg)
{
    _ident -= IDENT_SIZE;
    log("EXIT " + msg);
}

std::shared_ptr<Logger> Logger::logger()
{
    if (!LOGGER)
    {
        LOGGER = std::make_shared<Logger>();
    }

    return LOGGER;
}

#endif // DEBUG