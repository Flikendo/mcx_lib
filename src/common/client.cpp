#include "api/client.h"
#include "common/configuration/config_manager.hpp"

/**
 * Internal representation of the opaque C API client handle.
 * Owns the configuration manager shared by components that need configuration.
 */
struct client_handler {
    ConfigManager configManager;
    Logger logger;
};

/**
 * Starts the library
 * @return mcx_client_t The MCX client struct
 */
client_handler_t* init(void) {
    return new client_handler{};
}

/**
 * Detroys the library
 * @param client The MCX client struct
 * @return status_code_t STATUS_CODE_ERROR when error, otherwise STATUS_CODE_OK
 */
status_code_t deinit(client_handler_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERROR;
    }

    client->logger.log(LogLevel::DEBUG, "Deinit client OK");
    delete client;
    return STATUS_CODE_OK;
}

/**
 * Loads INI config from path file.
 * @param path Path to the INI file containing the configuration sections.
 * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
 */
status_code_t loadIniFileFromPath(IN client_handler_t* client, const char* pathFile) {
    if (client == nullptr ) {
        return STATUS_CODE_ERROR;
    }

    if (pathFile == nullptr) {
        client->logger.log(LogLevel::ERROR, "Loading INI file from path");
        return STATUS_CODE_ERROR;
    }

    try {
        client->logger.log(LogLevel::DEBUG, "Loading INI file from path");
        return client->configManager.loadConfig(pathFile, true);
    } catch (const std::exception& error) {
        client->logger.log(LogLevel::ERROR, error.what());
        return STATUS_CODE_ERROR;
    }
}

/**
 * Loads INI config from string.
 * @param iniConfig config already read from string.
 * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
 */
status_code_t loadIniFileFromString(IN client_handler_t* client, const char* iniConfig) {
    if (client == nullptr) {
        return STATUS_CODE_ERROR;
    }
    
    if (iniConfig == nullptr) {
        client->logger.log(LogLevel::ERROR, "Loading INI file from string");        
        return STATUS_CODE_ERROR;
    }

    try {
        client->logger.log(LogLevel::DEBUG, "Loading INI file from string");
        return client->configManager.loadConfig(iniConfig, false);
    } catch (const std::exception& error) {
        client->logger.log(LogLevel::ERROR, error.what());
        return STATUS_CODE_ERROR;
    }
}