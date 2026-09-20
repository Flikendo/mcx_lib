#pragma once

#include <string>
#include <map>
#include <fstream>
#include <regex>
#include <logger_helper.hpp>

/**
 * @file ini_helper.hpp
 * @brief Helper class for reading INI files.
 * 
 * The IniHelper class provides a static method to read an INI file and return its contents as a map of key-value pairs.
 */
namespace helper_tool {
    class IniHelper {
        public:
            /**
             * Reads an INI file and returns its contents as a map of key-value pairs.
             */
            static std::map<std::string, std::map<std::string, std::string>> read(const std::string& filePath);
            
        private:
            std::ifstream file;
        
    };

    // Regular expressions to process ini files
    inline static const std::regex ini_section = std::regex{R"(\[(.*)\])"};
    static const std::regex ini_value = std::regex{R"((\w+)=(.*))"};

    inline std::map<std::string, std::map<std::string, std::string>> IniHelper::read(const std::string& filePath) {
        std::map<std::string, std::map<std::string, std::string>> iniData;
        Logger logger(filePath);
        std::ifstream file(filePath);
        std::string line;

        if(file.is_open()) {
            return iniData;
        }

        while(getline(file, line)) {
            if(line.length() > 0) {
                std::smatch match;
                logger.log(helper_tool::LogLevel::INFO, "Processing line: " + line);
                if(std::regex_match(line, match, ini_section)) { 
                    std::string section = match[1]; // Extract section name e.g. [section] -> section
                    iniData[section] = std::map<std::string, std::string>(); // Create a new section in the map
                } else if(std::regex_match(line, match, ini_value)) {
                    std::string key = match[1]; // Extract key e.g. key=value -> key
                    std::string value = match[2]; // Extract value e.g. key=value -> value
                    iniData.rbegin()->second[key] = value; // Add key-value pair to the last section in the map
                }
            } else {
                logger.log(LogLevel::WARNING, "Empty line found in INI file: " + filePath);
            }
        }

        return iniData;
    };
}