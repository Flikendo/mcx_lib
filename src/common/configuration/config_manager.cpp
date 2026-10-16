#include "common/configuration/config_manager.hpp"
#include "common/configuration/config_constants.hpp"

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
    idmsConfig.populateFromIniFile(path, IDENTITY_SECTION_NAME);
    sipConfig.populateFromIniFile(path, IDENTITY_SECTION_NAME);
    mcxConfig.populateFromIniFile(path, IDENTITY_SECTION_NAME);
}