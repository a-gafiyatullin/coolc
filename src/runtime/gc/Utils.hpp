#pragma once

#include <string>

/**
 * @brief Get string representation of the time
 *
 * @param millis Time in millis
 * @return std::string Nice string for this time
 */
std::string printable_time(unsigned long long millis);

/**
 * @brief Get string representation of the size
 *
 * @param bytes Size in bytes
 * @return std::string Nice string for this bytes
 */
std::string printable_size(unsigned long long bytes);

/**
 * @brief String with size to size in bytes
 *
 * @param str String with size
 * @return size_t Size in bytes
 */
size_t str_to_size(const std::string &str);
