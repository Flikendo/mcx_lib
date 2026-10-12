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
             */
            void populateFromIniFile(const std::string& iniFilePath, const std::string& sectionName) {
                const auto iniData = helper_tool::IniHelper::read(iniFilePath);
                if (iniData.empty()) {
                    logger.log(LogLevel::ERROR, "Failed to read INI file or it was empty: " + iniFilePath);
                    throw std::runtime_error("Failed to read INI file or it was empty: " + iniFilePath);
                }

                if (iniData.find(sectionName) == iniData.end()) {
                    logger.log(LogLevel::ERROR, "Section " + sectionName + " not found in INI file: " + iniFilePath);
                    throw std::runtime_error("Section " + sectionName + " not found in INI file: " + iniFilePath);
                }
                
                logger.log(LogLevel::INFO, "Populating configuration from INI file: " + iniFilePath + ", section: " + sectionName);
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