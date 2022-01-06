#pragma once

#ifdef DEBUG
#include <iostream>
#include <memory>

/**
 * @brief Loggers hierarhy
 *
 */
class Logger
{
  private:
    static constexpr int IDENT_SIZE = 4;

    int _ident;
    std::shared_ptr<Logger> _parent_logger;

    int update_ident();

  public:
    /**
     * @brief Construct a new Logger object
     *
     */
    Logger() : _ident(0), _parent_logger(nullptr)
    {
    }

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

    /**
     * @brief Set the parent logger object
     *
     * @param parent parent logger
     */
    inline void set_parent_logger(const std::shared_ptr<Logger> &logger)
    {
        _parent_logger = logger;
    }
};
#endif // DEBUG