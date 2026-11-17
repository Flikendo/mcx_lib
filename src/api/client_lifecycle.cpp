#include "api/client_lifecycle.h"
#include "client_internal.hpp"

#include <memory>

client_handle_t* init(void) {
    try {
        auto client = std::make_unique<client_handle>();
        client->sipClient = std::make_unique<sip::ClientImpl>();
        return client.release();
    } catch (...) {
        return nullptr;
    }
}

status_code_t deinit(client_handle_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERR_NULL;
    }

    try {
        client->logger.log(LogLevel::DEBUG, "Deinit client");
        delete client;
        return STATUS_CODE_OK;
    } catch (...) {
        delete client;
        return STATUS_CODE_ERR_INTERNAL;
    }
}

status_code_t startClient(IN client_handle_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERR_NULL;
    }

    if (client->sipClient == nullptr) {
        return STATUS_CODE_ERR_STATE;
    }

    try {
        return client->sipClient->start() == PJ_SUCCESS
            ? STATUS_CODE_OK
            : STATUS_CODE_ERR_SIP;
    } catch (...) {
        return STATUS_CODE_ERR_INTERNAL;
    }
}

status_code_t stopClient(IN client_handle_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERR_NULL;
    }

    if (client->sipClient == nullptr) {
        return STATUS_CODE_ERR_STATE;
    }

    try {
        return client->sipClient->stop() == PJ_SUCCESS
            ? STATUS_CODE_OK
            : STATUS_CODE_ERR_SIP;
    } catch (...) {
        return STATUS_CODE_ERR_INTERNAL;
    }
}