#pragma once

#include "helper_tool/def_helper.hpp"
#include "helper_tool/logger_helper.hpp"
#include <string>

class ConfigBase {
    public:
        /**
         * Destructor for the ConfigBase class. Cleans up any resources used by the configuration settings.
         */
        virtual ~ConfigBase() = default;
        
        /**
         * Loads the configuration settings from an INI file.
         * @param iniFilePath The path to the INI file.
         * @param sectionName The name of the section in the INI file to read.
         * @throws std::runtime_error If the file is empty/unreadable or the section is missing.
         */
        void loadFromIniFile(const std::string& iniFilePath, const std::string& sectionName);

        /**
         * Populates this configuration from already parsed INI data.
         * @param iniData The nested map containing the INI data.
         * @param sectionName The name of the section in the INI data to read.
         * @throws std::runtime_error If the data is empty or the section is missing.
         */
        void loadFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName);

        /**
         * Returns a string representation of the configuration settings.
         * @return A string containing the configuration settings in a human-readable format.
         */
        virtual std::string toString() const = 0;

    protected:
        /**
         * Populates the configuration settings from the provided INI data.
         * @param iniData The nested map containing the INI data.
         * @param sectionName The name of the section in the INI data to read.
         */
        virtual void populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) = 0;

        /**
         * Logger instance for logging configuration-related messages.
         * This can be used to log errors, warnings, or informational messages related to configuration processing
         */
        Logger logger;
};