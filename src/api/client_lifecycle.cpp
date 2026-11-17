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

client_handle_t* initClientHandle(void) {
    try {
        client_handle_t* client = new client_handle_t();
        client->logger = new Logger();
        client->mcxPool = new mcx_pool_t();
        pool = client->mcxPool;
        return client;
    } catch (...) {
        return nullptr;
    }
}

status_code_t deinitClientHandle(client_handle_t* client) {
    if (client == nullptr) {
        return STATUS_CODE_ERR_NULL;
    }

    try {
        client->logger->log(LogLevel::DEBUG, "Deinit client");
        delete client;
        return STATUS_CODE_OK;
    } catch (...) {
        delete client;
        return STATUS_CODE_ERR_INTERNAL;
    }
}

status_code_t startClient(IN client_handle_t* client) {
    /* Must init PJLIB first: */
    pj_status_t status = pj_init();    
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, STATUS_CODE_ERROR);

    if (status != PJ_SUCCESS) {
        return STATUS_CODE_ERROR;
    }

    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, STATUS_CODE_ERROR);

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&client->mcxPool->cp, &pj_pool_factory_default_policy, 0);

    status = pjsip_endpt_create(&client->mcxPool->cp.factory, pj_gethostname()->ptr, &globalSipEndpoint);

    PJ_ASSERT_RETURN(status == PJ_SUCCESS, STATUS_CODE_ERROR);

    if (status != PJ_SUCCESS) {
        return STATUS_CODE_ERROR;
    }

    return STATUS_CODE_OK;
}

status_code_t stopClient(IN client_handle_t* client) {
  
}