#include "api/api_mcx.h"
#include "common/configuration/config_manager.hpp"

/**
 * Internal representation of the opaque C API client handle.
 * Owns the configuration manager shared by components that need configuration.
 */
struct mcx_client_handler {
    ConfigManager configManager;
};

/**
 * Starts the library
 * @return mcx_client_t The MCX client struct
 */
mcx_client_handler_t* mcx_client_init(void) {
    return new mcx_client_handler{};
}

/**
 * Detroys the library
 * @param mcx_client The MCX client struct
 * @return status_code_t returns STATUS_CODE_ERROR when error, otherwise STATUS_CODE_OK
 */
status_code_t mcx_client_deinit(mcx_client_handler_t* mcx_client) {
    if (mcx_client == nullptr) {
        return STATUS_CODE_ERROR;
    }
    
    delete mcx_client;
    return STATUS_CODE_OK;
}