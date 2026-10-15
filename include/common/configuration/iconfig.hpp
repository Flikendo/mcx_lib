#pragma once

#include <ini_helper.hpp>
#include <logger_helper.hpp>
#include <stdexcept>
#include <string>

namespace common {
    struct IConfig {
        public:
            /**
             * Destructor for the Config class. Cleans up any resources used by the configuration settings.
             */
            virtual ~IConfig() = default;
            
            /**
             * Populates the configuration settings from an INI file.
             * @param iniFilePath The path to the INI file.
             * @param sectionName The name of the section in the INI file to read.
             * @param config The configuration object to populate.
             * @throws std::runtime_error If the file is empty/unreadable or the section is missing.
             */
            void populateFromIniFile(const std::string& iniFilePath, const std::string& sectionName) {
                const auto iniData = helper_tool::IniHelper::read(iniFilePath);
                if (iniData.empty()) {
                    const std::string message =
                        "Failed to read INI file or it was empty: " + iniFilePath;
                    logger.log(LogLevel::ERROR, message);
                    throw std::runtime_error(message);
                }

                if (iniData.find(sectionName) == iniData.end()) {
                    const std::string message =
                        "Section " + sectionName + " not found in INI file: " + iniFilePath;
                    logger.log(LogLevel::ERROR, message);
                    throw std::runtime_error(message);
                }
                
                populateFromIniData(iniData, sectionName);
            }

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

        protected:
            /**
             * Logger instance for logging configuration-related messages.
             * This can be used to log errors, warnings, or informational messages related to configuration processing
             */
            Logger logger;
    };
}