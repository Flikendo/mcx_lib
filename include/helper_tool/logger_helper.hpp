#pragma once

#include <fstream>
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
         * Constructor: Initializes the logger with a specified log file name.
         * @param filename The name of the log file to write log messages to.
         */
        Logger(const std::string& filename);

        /**
         * Sets the minimum log level for the logger.
         * @param level The minimum log level to display.
         */
        static void setMinimumLogLevel(LogLevel level);

        /**
         * Parses a string representation of a log level and returns the corresponding LogLevel enum value.
         * @param name The string representation of the log level (e.g., "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL").
         * @return The corresponding LogLevel enum value.
         */
        static LogLevel parseLogLevel(const std::string& name);

        /**
         * Logs a message with the specified log level.
         * @param level The log level of the message.
         */
        void log(LogLevel level, const std::string& message);

    private:
        /**
         * Returns the current minimum log level for the logger.
         * @return The current minimum log level.
         */
        static LogLevel minimumLogLevel();

        /**
         * Returns a reference to the static storage for the minimum log level.
         * This allows the minimum log level to be shared across all instances of the Logger class.
         */
        static LogLevel& minimumLogLevelStorage();

        /**
         * Converts a LogLevel enum value to its string representation.
         * @param level The LogLevel enum value to convert.
         * @return The string representation of the log level.
         */
        std::string levelToString(LogLevel level);

    private:
        /**
         * The file stream for the log file.
         */
        std::ofstream logFile;
};