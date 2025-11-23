#include "util/logging.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace trdp::util
{
namespace
{
std::mutex &logMutex()
{
    static std::mutex mutex;
    return mutex;
}

std::string levelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
}

std::string formatTimestamp(std::chrono::system_clock::time_point ts)
{
    const auto timeT = std::chrono::system_clock::to_time_t(ts);
    const auto tm = *std::localtime(&timeT);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%F %T");
    return oss.str();
}

void log(LogLevel level, const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex());
    std::cout << "[" << levelToString(level) << "] " << formatTimestamp(std::chrono::system_clock::now()) << " - "
              << message << std::endl;
}

} // namespace trdp::util
