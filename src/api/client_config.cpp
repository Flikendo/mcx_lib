#include "api/client_config.h"
#include "client_internal.hpp"

#include <exception>

status_code_t loadIniFile(
    client_handler_t* client,
    const char* iniConfig,
    bool isFromPath) {
    if (client == nullptr) {
        return STATUS_CODE_ERROR;
    }

    if (iniConfig == nullptr) {
        client->logger.log(LogLevel::ERROR, "INI configuration is null");
        return STATUS_CODE_ERR_CONFIG;
    }

    try {
        client->logger.log(LogLevel::DEBUG, "Loading INI configuration");
        return client->configManager.loadConfig(iniConfig, isFromPath);
    } catch (const std::exception& error) {
        client->logger.log(LogLevel::ERROR, error.what());
        return STATUS_CODE_ERROR;
    }
}