#include "api/client_lifecycle.h"
#include "client_internal.hpp"

client_handler_t* init(void) {
    return new client_handler{};
}

status_code_t deinit(client_handler_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERROR;
    }

    client->logger.log(LogLevel::DEBUG, "Deinit client OK");
    delete client;
    return STATUS_CODE_OK;
}