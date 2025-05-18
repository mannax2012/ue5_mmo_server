#include "Log.h"
LogLevel GLOBAL_LOG_LEVEL = LogLevel::DEBUG;

LogLevel LogLevelFromString(const std::string& s) {
    if (s == "DEBUG") return LogLevel::DEBUG;
    if (s == "INFO") return LogLevel::INFO;
    if (s == "WARNING") return LogLevel::WARNING;
    if (s == "ERROR") return LogLevel::ERROR;
    if (s == "NONE") return LogLevel::NONE;
    return LogLevel::DEBUG;
}
