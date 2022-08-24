#include "Utils.hpp"

#define SECONDS 1000
#define MINUTES (SECONDS * 60)
#define HOURS (MINUTES * 60)

std::string printable_time(unsigned long long time_in_millis)
{
    std::string time;

    int hours = time_in_millis / HOURS;
    int minutes = time_in_millis % HOURS / MINUTES;
    int seconds = time_in_millis % MINUTES / SECONDS;
    int millis = time_in_millis % SECONDS;

    if (hours > 0)
    {
        time += std::to_string(hours) + "h:";
    }

    if (minutes > 0)
    {
        time += std::to_string(minutes) + "m:";
    }

    time += std::to_string(seconds) + ".";
    time += std::to_string(millis % 1000) + "s";

    return time;
}

#define Kb 1024
#define Mb (Kb * 1024)
#define Gb (Mb * 1024)

std::string printable_size(unsigned long long size_in_bytes)
{
    std::string size;

    int gbytes = size_in_bytes / Gb;
    int mbytes = (size_in_bytes % Gb) / Mb;
    int kbytes = (size_in_bytes % Mb) / Kb;
    int bytes = size_in_bytes % Kb;

    if (gbytes > 0)
    {
        size += std::to_string(gbytes) + "Gb ";
    }

    if (mbytes > 0)
    {
        size += std::to_string(mbytes) + "Mb ";
    }

    size += std::to_string(kbytes) + "Kb ";
    size += std::to_string(bytes % 1000) + "b";

    return size;
}

#define str_to_size_for(var, str)                                                                                      \
    suffix = str.find(#var);                                                                                           \
    if (suffix != std::string::npos)                                                                                   \
    {                                                                                                                  \
        return std::stoi(str.substr(0, suffix)) * var;                                                                 \
    }

size_t str_to_size(const std::string &str)
{
    int suffix = 0;

    str_to_size_for(Gb, str);
    str_to_size_for(Mb, str);
    str_to_size_for(Kb, str);

    return std::stoi(str);
}