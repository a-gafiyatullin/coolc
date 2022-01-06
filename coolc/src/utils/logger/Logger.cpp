#include "utils/logger/Logger.h"

#ifdef DEBUG
std::shared_ptr<Logger> Logger::LOGGER = nullptr;

void Logger::log(const std::string &msg)
{
    std::cout << std::string(_ident, ' ') << msg << std::endl;
}

void Logger::log_enter(const std::string &msg)
{
    _ident += IDENT_SIZE;
    log("Enter " + msg);
}

void Logger::log_exit(const std::string &msg)
{
    log("Exit " + msg);
    _ident -= IDENT_SIZE;
}

std::shared_ptr<Logger> Logger::get_logger()
{
    if (!LOGGER)
    {
        LOGGER = std::make_shared<Logger>();
    }

    return LOGGER;
}

#endif // DEBUG