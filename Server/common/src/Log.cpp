#include "Log.h"

//DEFINED HERE BEcause otherwise it messes up with the include order
LogLevel GLOBAL_LOG_LEVEL = LogLevel::DEBUG;
LogLevel GLOBAL_FILE_LOG_LEVEL = LogLevel::DEBUG;
bool LOG_TO_FILE = false;
std::string LOG_FILE_DIR = "./logs";
std::string LOG_FILE_NAME = "server.log";
