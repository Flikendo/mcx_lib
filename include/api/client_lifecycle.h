#ifndef MCX_API_CLIENT_LIFECYCLE_H
#define MCX_API_CLIENT_LIFECYCLE_H

#include "api/data/definitions.h"
#include "api/data/types.h"

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * Represents the client endpoint structure.
     */
    typedef struct client_endpoint client_endpoint_t;

    /**
     * Initialize a client endpoint.
     * @return client_endpoint_t object, or NULL if creation fails.
     */
    MCX_API OUT client_endpoint_t* CALLCLIENT create_client_endpoint(IN void);

    /**
     * Destroys the client endpoint.
     * @param client The client endpoint.
     * @return STATUS_CODE_OK on success, otherwise an error status.
     */
    MCX_API OUT status_code_t CALLCLIENT destroy_client_endpoint(IN client_endpoint_t* client);
    
    /**
     * Starts the client endpoint.
     * @param client The client endpoint.
     * @return STATUS_CODE_OK on success, otherwise an error status.
     */
    MCX_API OUT status_code_t CALLCLIENT start_client(IN client_endpoint_t* client);

    /**
     * Stops the client endpoint.
     * @param client The client endpoint.
     * @return STATUS_CODE_OK on success, otherwise an error status.
     */
    MCX_API OUT status_code_t CALLCLIENT stop_client(IN client_endpoint_t* client);
#ifdef __cplusplus
}
#endif

#endif
