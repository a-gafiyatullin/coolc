#pragma once

#ifdef DEBUG
#include <iostream>
#include <memory>

class Logger
{
  private:
    static constexpr int IDENT_SIZE = 4;
    static std::shared_ptr<Logger> LOGGER;

    int _ident;

  public:
    // TODO: it is public for now
    Logger() : _ident(0)
    {
    }

    /**
     * @brief Get the logger
     *
     * @return Logger instance
     */
    static std::shared_ptr<Logger> get_logger();

    /**
     * @brief Write log message with current indentation
     *
     * @param msg Message
     */
    void log(const std::string &msg);

    /**
     * @brief Write log message with deeper indentation
     *
     * @param msg Message
     */
    void log_enter(const std::string &msg);

    /**
     * @brief Write log message with less indentation
     *
     * @param msg Message
     */
    void log_exit(const std::string &msg);
};
#endif // DEBUG