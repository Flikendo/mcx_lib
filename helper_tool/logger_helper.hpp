#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Enum to represent log levels
enum LogLevel { 
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
    public:
        /**
         * Default constructor: Initializes the logger without a log file.
         * Log messages will only be output to the console.
         */
        Logger() = default;

        /**
         * Sets the minimum log level for the logger.
         * @param level The minimum log level to display.
         */
        static void setMinimumLogLevel(LogLevel level)
        {
            minimumLogLevelStorage() = level;
        }

        /**
         * Parses a string representation of a log level and returns the corresponding LogLevel enum value.
         * @param name The string representation of the log level (e.g., "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL").
         * @return The corresponding LogLevel enum value.
         */
        static LogLevel parseLogLevel(const std::string& name)
        {
            if (name == "DEBUG") return DEBUG;
            if (name == "INFO") return INFO;
            if (name == "WARNING") return WARNING;
            if (name == "ERROR") return ERROR;
            if (name == "CRITICAL") return CRITICAL;

            throw std::invalid_argument(
                "Invalid log level '" + name
                + "'. Expected DEBUG, INFO, WARNING, ERROR, or CRITICAL.");
        }

        /**
         * Constructor: Initializes the logger with a specified log file name.
         * @param filename The name of the log file to write log messages to.
         */
        Logger(const std::string& filename)
        {
            logFile.open(filename, std::ios::app);
            if (!logFile.is_open()) {
                std::cerr << "Error opening log file." << std::endl;
            }
        }

        /**
         * Destructor: Closes the log file stream when the Logger object is destroyed.
         */
        ~Logger() {
            logFile.close(); 
        }

        /**
         * Logs a message with the specified log level.
         * @param level The log level of the message.
         */
        void log(LogLevel level, const std::string& message)
        {
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

    private:
        /**
         * Returns the current minimum log level for the logger.
         * @return The current minimum log level.
         */
        static LogLevel minimumLogLevel()
        {
            return minimumLogLevelStorage();
        }

        /**
         * Returns a reference to the static storage for the minimum log level.
         * This allows the minimum log level to be shared across all instances of the Logger class.
         */
        static LogLevel& minimumLogLevelStorage()
        {
            static LogLevel level = INFO;
            return level;
        }

        /**
         * Converts a LogLevel enum value to its string representation.
         * @param level The LogLevel enum value to convert.
         * @return The string representation of the log level.
         */
        std::string levelToString(LogLevel level)
        {
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

    private:
        /**
         * The file stream for the log file.
         */
        std::ofstream logFile;


};