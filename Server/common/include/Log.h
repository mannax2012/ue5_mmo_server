#pragma once
#include <iostream>
#include <string>

// Log levels
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    NONE = 4
};

extern LogLevel GLOBAL_LOG_LEVEL;

inline void SetLogLevel(LogLevel level) {
    GLOBAL_LOG_LEVEL = level;
}

inline const char* LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "LOG";
    }
}

// ANSI color codes for log levels
inline const char* LogLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "\033[36m";    // Cyan
        case LogLevel::INFO: return "\033[32m";     // Dark-Green
        case LogLevel::WARNING: return "\033[33m";  // Yellow
        case LogLevel::ERROR: return "\033[31m";    // Red
        case LogLevel::NONE: return "\033[92m";      // Light GREEN
        default: return "\033[0m";                  // Reset
    }
}

inline void Log(LogLevel level, const std::string& msg) {
    if (level < GLOBAL_LOG_LEVEL) return;
    std::ostream& out = (level >= LogLevel::WARNING) ? std::cerr : std::cout;
    out << LogLevelColor(level) << "[" << LogLevelToString(level) << "] " << msg << "\033[0m" << std::endl;
}

// Set log level from string (e.g. from config)
LogLevel LogLevelFromString(const std::string& s);

#define LOG_DEBUG(msg)   Log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg)    Log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg)   Log(LogLevel::ERROR, msg)
#define LOG_CONFIG(msg)   Log(LogLevel::CONFIG, msg)
#define LOG(msg)  Log(LogLevel::NONE, msg)
