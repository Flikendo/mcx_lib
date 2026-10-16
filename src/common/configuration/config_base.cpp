#include "common/configuration/config_base.hpp"

#include "helper_tool/def_helper.hpp"
#include "helper_tool/ini_helper.hpp"
#include "helper_tool/logger_helper.hpp"
#include <stdexcept>
#include <string>
        
void ConfigBase::populateFromIniFile(const std::string& iniFilePath, const std::string& sectionName) {
    const auto iniData = helper_tool::IniHelper::read(iniFilePath);
    if (iniData.empty()) {
        const std::string message = "Failed to read INI file or it was empty: " + iniFilePath;
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
        
        return;
    }

    if (iniData.find(sectionName) == iniData.end()) {
        const std::string message = "Section " + sectionName + " not found in INI file: " + iniFilePath;
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
        
        return;
    }
    
    populateFromIniData(iniData, sectionName);
}
