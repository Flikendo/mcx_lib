#ifndef MCX_API_CLIENT_LIFECYCLE_H
#define MCX_API_CLIENT_LIFECYCLE_H

#include "api/data/definitions.h"
#include "api/data/types.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct client_heandler client_handler_t;

/**
 * Creates a client handle.
 * @return client_handler_t object, or NULL if creation fails.
 */
MCX_API OUT client_handler_t* CALLCLIENT init(IN void);

/**
 * Destroys the client handle.
 * @param client The client handle.
 * @return STATUS_CODE_OK on success, otherwise an error status.
 */
MCX_API OUT status_code_t CALLCLIENT deinit(IN client_handler_t* client);
#ifdef __cplusplus
}
#endif

#endif
