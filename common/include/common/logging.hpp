#pragma once

#include <iostream>
#include <string_view>

namespace common {

enum class LogLevel {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3,
};

inline LogLevel global_log_level = LogLevel::Info;

inline void set_log_level(LogLevel level) {
    global_log_level = level;
}

inline const char* level_to_string(LogLevel level) {
    switch (level) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

template <typename T>
inline void log(LogLevel level, const T& message) {
    if (static_cast<int>(level) < static_cast<int>(global_log_level)) {
        return;
    }
    std::clog << "[" << level_to_string(level) << "] " << message << '\n';
}

} // namespace common

// Simple convenience macros
#define LOG_DEBUG(msg) ::common::log(::common::LogLevel::Debug, (msg))
#define LOG_INFO(msg)  ::common::log(::common::LogLevel::Info,  (msg))
#define LOG_WARN(msg)  ::common::log(::common::LogLevel::Warn,  (msg))
#define LOG_ERROR(msg) ::common::log(::common::LogLevel::Error, (msg))

