#pragma once

#ifdef DEBUG
#include <memory>

class Logger
{
  private:
    static constexpr int IDENT_SIZE = 4;
    static std::shared_ptr<Logger> LOGGER;

    int _ident;

  public:
    Logger() : _ident(0) {}

    /**
     * @brief Get the logger
     *
     * @return Logger instance
     */
    static std::shared_ptr<Logger> logger();

    /**
     * @brief Write log message with current indentation
     *
     * @param msg Message
     * @param cr Do carriage return
     * @param do_ident Make indentation before message
     */
    void log(const std::string &msg, const bool &cr = true, const bool &do_ident = true);

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

#define LOG(msg) Logger::logger()->log(msg);
#define LOG_NO_CR(msg) Logger::logger()->log(msg, false);
#define LOG_NO_CR_NO_IDENT(msg) Logger::logger()->log(msg, false, false);
#define LOG_NO_IDENT(msg) Logger::logger()->log(msg, true, false);
#define LOG_ENTER(msg) Logger::logger()->log_enter(msg);
#define LOG_EXIT(msg) Logger::logger()->log_exit(msg);
#else
#define LOG(msg)
#define LOG_NO_CR(msg)
#define LOG_NO_CR_NO_IDENT(msg)
#define LOG_NO_IDENT(msg)
#define LOG_ENTER(msg)
#define LOG_EXIT(msg)
#endif // DEBUG
