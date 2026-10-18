#include "common/configuration/config_base.hpp"
#include "common/configuration/config_constants.hpp"
#include "helper_tool/ini_helper.hpp"
#include "helper_tool/logger_helper.hpp"

#include <stdexcept>

void ConfigBase::loadFromIniFile(const std::string& iniFilePath, const std::string& sectionName) {
    const auto iniData = helper_tool::IniHelper::read(iniFilePath);
    if (iniData.empty()) {
        const std::string message = "Failed to read INI file or it was empty: " + iniFilePath;
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
    }

    loadFromIniData(iniData, sectionName);
}

void ConfigBase::loadFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) {
    if (iniData.empty()) {
        const std::string message = "Cannot populate configuration from empty INI data";
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
    }

    if (iniData.find(sectionName) == iniData.end()) {
        const std::string message = "INI section not found: " + sectionName;
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
    }
    
    configureMinimumLogLevel(iniData);
    populateFromIniData(iniData, sectionName);
}

void ConfigBase::configureMinimumLogLevel(const DefHelper::ConfigMap& iniData) {
    std::string minimumConfigLog = helper_tool::IniHelper::getStringValue(iniData, LOG_SECTION_NAME, MINIMUN_CONFIG_LOG);
    LogLevel logLevel = logger.parseLogLevel(minimumConfigLog);
    logger.setMinimumLogLevel(logLevel);
}
