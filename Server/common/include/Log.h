#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

// Log levels
enum class LogLevel {
    DEBUG_EXTENDED,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    NONE
};

extern LogLevel GLOBAL_LOG_LEVEL;
extern bool LOG_TO_FILE;
extern std::string LOG_FILE_DIR;
extern LogLevel GLOBAL_FILE_LOG_LEVEL;
extern std::string LOG_FILE_NAME;

inline LogLevel LogLevelFromString(const std::string& s) {
    if (s == "DEBUG") return LogLevel::DEBUG;
    if(s== "DEBUG_EXTENDED") return LogLevel::DEBUG_EXTENDED;
    if (s == "INFO") return LogLevel::INFO;
    if (s == "WARNING") return LogLevel::WARNING;
    if (s == "ERROR") return LogLevel::ERROR;
    if (s == "NONE") return LogLevel::NONE;
    return LogLevel::DEBUG;
}

inline void SetLogFileName(const std::string& name) {
    LOG_FILE_NAME = name;
}
inline std::string GetLogFileName() {
    return LOG_FILE_NAME;
}
inline void SetLogLevel(LogLevel level) {
    GLOBAL_LOG_LEVEL = level;
}

inline void SetFileLogLevel(LogLevel level) {
    GLOBAL_FILE_LOG_LEVEL = level;
}
inline void SetLogToFile(bool enable) {
    LOG_TO_FILE = enable;
}
inline void SetLogFileDir(const std::string& dir) {
    LOG_FILE_DIR = dir;
}
inline const char* LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::DEBUG_EXTENDED: return "DEBUG_EXTENDED";
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
     // Optional file logging
    if (LOG_TO_FILE && level >= GLOBAL_FILE_LOG_LEVEL) {
        try {
            std::filesystem::create_directories(LOG_FILE_DIR);
            std::ofstream ofs(LOG_FILE_DIR + "/" + LOG_FILE_NAME, std::ios::app);
            if (ofs.is_open()) {
                // Write timestamped log line (no color codes)
                std::time_t t = std::time(nullptr);
                std::tm tm;
                localtime_r(&t, &tm);
                char timebuf[32];
                std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);
                ofs << timebuf << " [" << LogLevelToString(level) << "] " << msg << std::endl;
            }
        } catch (...) {
            // Ignore file logging errors
        }
    }
    if (level < GLOBAL_LOG_LEVEL) return;
    std::ostream& out = (level >= LogLevel::WARNING) ? std::cerr : std::cout;
    out << LogLevelColor(level) << "[" << LogLevelToString(level) << "] " << msg << "\033[0m" << std::endl;
}

// Set log level from string (e.g. from config)
LogLevel LogLevelFromString(const std::string& s);

#define LOG_DEBUG(msg)   Log(LogLevel::DEBUG, msg)
#define LOG_DEBUG_EXT(msg)   Log(LogLevel::DEBUG_EXTENDED, msg)
#define LOG_INFO(msg)    Log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg)   Log(LogLevel::ERROR, msg)
#define LOG_CONFIG(msg)   Log(LogLevel::INFO, msg)
#define LOG(msg)  Log(LogLevel::NONE, msg)
