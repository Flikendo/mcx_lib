#pragma once

#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace helper_tool {
    class IniHelper {
    public:
        /** Reads an INI file and returns its contents as a nested map. 
         * @param filePath The path to the INI file.
         * @return A map where the keys are section names and the values are maps of key-value pairs within those sections.
        */
        static std::map<std::string, std::map<std::string, std::string>> read(const std::string& filePath);

    private:
        /**
         * Trims leading and trailing whitespace from a string.
         * @param value The string to trim.
         */
        static std::string trim(const std::string& value);
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

    inline std::map<std::string, std::map<std::string, std::string>> IniHelper::read(const std::string& filePath) {
        std::map<std::string, std::map<std::string, std::string>> iniData;
        std::ifstream file(filePath);
        std::string line;
        std::string currentSection;

        if (!file.is_open()) {
            return iniData;
        }

        while (std::getline(file, line)) {
            std::string trimmed = trim(line);

            if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
                continue;
            }

            if (trimmed.front() == '[' && trimmed.back() == ']') {
                currentSection = trim(trimmed.substr(1, trimmed.size() - 2));
                iniData[currentSection] = {};
                continue;
            }

            const std::size_t equalPos = trimmed.find('=');
            if (equalPos == std::string::npos || currentSection.empty()) {
                continue;
            }

            const std::string key = trim(trimmed.substr(0, equalPos));
            const std::string value = trim(trimmed.substr(equalPos + 1));
            iniData[currentSection][key] = value;
        }

        return iniData;
    }
}