#pragma once

#include "api/data/types.h"
#include "api/data/definitions.h"

extern "C" {
    typedef struct mcx_client_handler mcx_client_handler_t;

    /**
     * Starts the library
     */
    OUT mcx_client_handler_t* CALLCLIENT mcx_client_init(IN void);

    /**
     * Detroys the library
     */
    OUT status_code_t CALLCLIENT mcx_client_deinit(IN mcx_client_handler_t* mcx_client);
}