#include "helper_tool/log_helper.hpp"

#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error opening log file." << std::endl;
    }
}

void Logger::setMinimumLogLevel(LogLevel level) {
    minimumLogLevelStorage() = level;
}

LogLevel Logger::parseLogLevel(const std::string& name) {
    if (name == "DEBUG") return DEBUG;
    if (name == "INFO") return INFO;
    if (name == "WARNING") return WARNING;
    if (name == "ERROR") return ERROR;
    if (name == "CRITICAL") return CRITICAL;

    throw std::invalid_argument(
        "Invalid log level '" + name
        + "'. Expected DEBUG, INFO, WARNING, ERROR, or CRITICAL.");
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minimumLogLevel()) {
        return;
    }

    // Get current timestamp
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp),
            "%Y-%m-%d %H:%M:%S", timeinfo);

    // Create log entry
    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] "
            << levelToString(level) << ": " << message
            << std::endl;

    // Output to console
    std::cout << logEntry.str();

    // Output to log file
    if (logFile.is_open()) {
        logFile << logEntry.str();
        logFile.flush(); // Ensure immediate write to file
    }
}

LogLevel Logger::minimumLogLevel() {
    return minimumLogLevelStorage();
}

LogLevel& Logger::minimumLogLevelStorage() {
    static LogLevel level = INFO;
    return level;
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
    case DEBUG:
        return "[DEBUG]";
    case INFO:
        return "[INFO]";
    case WARNING:
        return "[WARNING]";
    case ERROR:
        return "[ERROR]";
    case CRITICAL:
        return "[CRITICAL]";
    default:
        return "[UNKNOWN]";
    }
}