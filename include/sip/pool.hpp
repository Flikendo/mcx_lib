#pragma once

#include "api/data/types.h"

#include <pjsip.h>
#include <pjlib.h>

/** The mcx_pool structure encapsulates the PJSIP caching pool factory and the associated memory pool,
 * providing a convenient way to manage memory allocation and deallocation for SIP-related operations.
 * It is used in conjunction with the ClientImpl class to facilitate the creation and destruction of PJSIP pools.
 */
typedef struct mcx_pool {
    pj_caching_pool pool_factory;   /** Pointer to the PJSIP caching pool factory. */
    pj_pool_t* pool;                /** Pointer to the PJSIP memory pool. */
    bool pool_factory_initialized;  /** Flag indicating whether the pool factory is initialized. */
} mcx_pool_t;

/**
 * @file pool.hpp
 * @brief Defines the mcx_pool structure for managing PJSIP memory pools.   
 */
class Pool {
    public:
        /**
         * Creates PJSIP pool
         * @param client The client endpoint.
         * @return PJ_SUCCESS on success, otherwise an error status.
         */
        static pj_status_t create_mcx_pool(mcx_pool_t** pool);

        /**
         * Destroys PJSIP pool
         * @param client The client endpoint.
         * @return PJ_SUCCESS on success, otherwise an error status.
         */
        static pj_status_t destroy_mcx_pool();
};