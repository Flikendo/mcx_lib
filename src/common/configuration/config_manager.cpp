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

status_code_t ConfigManager::loadConfig(const std::string& config, bool isFromPath) {
    DefHelper::ConfigMap iniData;

    if (isFromPath) {
        iniData = IniHelper::readPath(config);
    } else {
        iniData = IniHelper::readConfigString(config);
    }

    if (iniData.empty()) {
        const std::string message = "Failed to read INI file or it was empty: " + config;
        logger.log(LogLevel::ERROR, message);
        
        return STATUS_CODE_ERR_CONFIG;
    }

    idmsConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    sipConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);
    mcxConfig.loadFromIniData(iniData, IDENTITY_SECTION_NAME);

    return STATUS_CODE_OK;
}