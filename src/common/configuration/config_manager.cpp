#include "common/configuration/config_manager.hpp"
#include "common/configuration/config_constants.hpp"
#include "helper_tool/ini_helper.hpp"

#include <stdexcept>
#include <string>

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

    idmsConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    sipConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    mcxConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
}