#pragma once

#include <fstream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include "logger_helper.hpp"
#include "def_helper.hpp"

/**
 * @file ini_helper.hpp
 * Helper class for reading INI files and retrieving configuration values.
 */
namespace helper_tool {
class IniHelper {
    public:
        /** Reads an INI file and returns its contents as a nested map. 
         * @param filePath The path to the INI file.
         * @return A map where the keys are section names and the values are maps of key-value pairs within those sections.
        */
        static DefHelper::ConfigMap read(const std::string& filePath);

        /** Gets a string value from the INI data.
         * @param iniData The nested map containing the INI data.
         * @param section The section name.
         * @param key The key within the section.
         * @return The value associated with the key, or an empty string if not found.
         */
        static std::string getStringValue(const DefHelper::ConfigMap& iniData, const std::string& section, const std::string& key);

        /**
         * Gets a value of type T from the INI data.
         * @tparam T The type of the value to retrieve. 
         * @param iniData The nested map containing the INI data.
         * @param section The section name.
         * @param key The key within the section.
         * @return The value associated with the key, converted to type T.
         * @throws std::out_of_range If the section or key is not found.
         * @throws std::invalid_argument or std::out_of_range If conversion to T fails.
         */
        template <typename T> static T getValue(const DefHelper::ConfigMap& iniData, const std::string& section, 
            const std::string& key);

    private:
        /**
         * Trims leading and trailing whitespace from a string.
         * @param value The string to trim.
         */
        static std::string trim(const std::string& value);

    private:
        /**
         * Logger instance for logging messages related to INI file processing.
         * This can be used to log errors, warnings, or informational messages during INI file reading and value retrieval.
         */
        inline static Logger logger;
    };

    inline std::string IniHelper::trim(const std::string& value) {
        const std::string whitespace = " \t\r\n"; 

        const std::size_t first = value.find_first_not_of(whitespace); 
        if (first == std::string::npos) {
            return "";
        }

        const std::size_t last = value.find_last_not_of(whitespace);
        return value.substr(first, last - first + 1);
    }

    inline DefHelper::ConfigMap IniHelper::read(const std::string& filePath) {
        DefHelper::ConfigMap iniData;
        std::ifstream file(filePath);
        std::string line;
        std::string currentSection;

        if (!file.is_open()) {
            return iniData;
        }

        while (std::getline(file, line)) {
            std::string trimmed = trim(line);

            if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') continue;

            if (trimmed.front() == '[' && trimmed.back() == ']') {
                currentSection = trim(trimmed.substr(1, trimmed.size() - 2));
                iniData[currentSection] = {};
                continue;
            }

            const std::size_t equalPos = trimmed.find('=');
            if (equalPos == std::string::npos || currentSection.empty()) continue;

            const std::string key = trim(trimmed.substr(0, equalPos));
            const std::string value = trim(trimmed.substr(equalPos + 1));
            iniData[currentSection][key] = value;
        }

        return iniData;
    }

    inline std::string IniHelper::getStringValue(const DefHelper::ConfigMap& iniData, const std::string& section,
         const std::string& key) {
        const auto sectionIt = iniData.find(section);
        if (sectionIt == iniData.end()) {
            logger.log(LogLevel::ERROR, "INI section not found: " + section);
            return "";
        }

        const auto keyIt = sectionIt->second.find(key);
        if (keyIt == sectionIt->second.end()) {
            logger.log(LogLevel::ERROR, "INI key not found: " + key);
            return "";
        }

        if (keyIt->second.empty()) {
            logger.log(LogLevel::WARNING, "INI key has value empty" + key);
        }

        return keyIt->second;
    }

    template <typename T> T IniHelper::getValue(const DefHelper::ConfigMap& iniData, const std::string& section,
        const std::string& key) {
        const auto sectionIt = iniData.find(section);
        if (sectionIt == iniData.end()) {
            throw std::out_of_range("INI section not found: " + section);
        }

        const auto valueIt = sectionIt->second.find(key);
        if (valueIt == sectionIt->second.end()) {
            throw std::out_of_range("INI key not found: " + section + "." + key);
        }

        const std::string& raw = valueIt->second;
        if (raw.empty()) {
            logger.log(LogLevel::WARNING, "INI key has value empty");
        }
        
        if constexpr (std::is_same_v<T, std::string>) {
            return raw;
        } else if constexpr (std::is_same_v<T, int>) {
            std::size_t parsedLength = 0;
            const int value = std::stoi(raw, &parsedLength);
            if (parsedLength != raw.size()) {
                throw std::invalid_argument("Invalid integer in INI key: " + section + "." + key);
            }
            return value;
        } else if constexpr (std::is_same_v<T, std::int64_t>) {
            std::size_t parsedLength = 0;
            const std::int64_t value = std::stoll(raw, &parsedLength);
            if (parsedLength != raw.size()) {
                throw std::invalid_argument("Invalid integer in INI key: " + section + "." + key);
            }
            return value;
        } else {
            logger.log(LogLevel::ERROR, "Unsupported type for INI value conversion.");
            static_assert(std::is_same_v<T, std::string> ||
                std::is_same_v<T, int> ||
                std::is_same_v<T, std::int64_t>,
                "getValue supports only std::string, int, and std::int64_t");
        }       
    }
}