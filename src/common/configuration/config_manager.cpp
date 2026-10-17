#include "common/configuration/config_manager.hpp"
#include "common/configuration/config_constants.hpp"
#include "helper_tool/ini_helper.hpp"

#include <stdexcept>
#include <string>
#include <utility>

const idms::Config& ConfigManager::getIdmsConfig() const {
    return idmsConfig;
}

const sip::Config& ConfigManager::getSipConfig() const {
    return sipConfig;
}

const mcx::Config& ConfigManager::getMcxConfig() const {
    return mcxConfig;
}

void ConfigManager::loadConfigFromIniFile(const std::string& path) {
    const auto iniData = helper_tool::IniHelper::read(path);

    if (iniData.empty()) {
        const std::string message = "Failed to read INI file or it was empty: " + path;
        logger.log(LogLevel::ERROR, message);
        throw std::runtime_error(message);
    }

    idms::Config nextIdmsConfig;
    sip::Config nextSipConfig;
    mcx::Config nextMcxConfig;

    nextIdmsConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    nextSipConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    nextMcxConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);

    idmsConfig = std::move(nextIdmsConfig);
    sipConfig = std::move(nextSipConfig);
    mcxConfig = std::move(nextMcxConfig);
}