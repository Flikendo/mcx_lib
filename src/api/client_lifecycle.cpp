#include "api/client_lifecycle.h"
#include "client_internal.hpp"

#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <memory>

/**
 * The PJSIP endpoint used for managing SIP sessions and communication.
 */
static pjsip_endpoint* globalSipEndpoint;

client_endpoint_t* create_client_endpoint(void) {
    try {
        client_endpoint_t* client = new client_endpoint_t();
        client->configManager = new ConfigManager();
        client->logger = new Logger();
        client->sipClient = std::make_unique<sip::ClientImpl>();
        return client;
    } catch (...) {
        return nullptr;
    }
}

status_code_t destroy_client_endpoint(client_endpoint_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERR_NULL;
    }

    try {
        client->logger->log(LogLevel::DEBUG, "Deinit client endpoint");
        delete client;
        return STATUS_CODE_OK;
    } catch (...) {
        delete client;
        return STATUS_CODE_ERR_INTERNAL;
    }
}

status_code_t start_client_endpoint(IN client_endpoint_t* client) {

}

status_code_t stop_client_endpoint(IN client_endpoint_t* client) {
  
}