#pragma once

#include "helper_tool/def_helper.hpp"
#include "helper_tool/log_helper.hpp"
#include "api/data/types.h"

#include <string>

class ConfigBase {
    public:
        virtual ~ConfigBase() = default;

        /**
         * Loads the configuration settings from an INI file.
         * @param iniFilePath The path to the INI file.
         * @param sectionName The name of the section in the INI file to read.
         * @throws std::runtime_error If the file is empty/unreadable or the section is missing.
         * @throws std::invalid_argument If a configuration value is invalid.
         */
        void loadFromIniFile(const std::string& iniFilePath, const std::string& sectionName);

        /**
         * Populates this configuration from already parsed INI data.
         * @param iniData The nested map containing the INI data.
         * @param sectionName The name of the section in the INI data to read.
         * @throws std::runtime_error If the data is empty or the section is missing.
         * @throws std::invalid_argument If a configuration value is invalid.
         */
        void loadFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName);

    protected:
        ConfigBase() = default;

        /**
         * Populates the configuration settings from the provided INI data.
         * @param iniData The nested map containing the INI data.
         * @param sectionName The name of the section in the INI data to read.
         */
        virtual status_code_t populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) = 0;

        /**
         * Configures the minimum log level for configuration
         * @param iniData The nested map containing the INI data.
         */
        void configureMinimumLogLevel(const DefHelper::ConfigMap& iniData);
       
        /**
         * Returns a string representation of the configuration settings.
         * @return A string containing the configuration settings in a human-readable format.
         */
        virtual std::string toString() const = 0;

        /**
         * Logger instance for logging messages related to configuration processing.
         * This can be used to log errors, warnings, or informational messages during configuration loading and processing.
         */
        Logger logger;
};