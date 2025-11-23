#pragma once

#include <chrono>
#include <string>

namespace trdp::util
{

enum class LogLevel
{
    Debug,
    Info,
    Warn,
    Error,
};

void log(LogLevel level, const std::string &message);

inline void logDebug(const std::string &message)
{
    log(LogLevel::Debug, message);
}

inline void logInfo(const std::string &message)
{
    log(LogLevel::Info, message);
}

inline void logWarn(const std::string &message)
{
    log(LogLevel::Warn, message);
}

inline void logError(const std::string &message)
{
    log(LogLevel::Error, message);
}

std::string formatTimestamp(std::chrono::system_clock::time_point ts);

} // namespace trdp::util
