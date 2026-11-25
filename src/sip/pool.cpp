#include "sip/pool.hpp"

static mcx_pool_t g_pool;  // Global instance of mcx_pool_t

pj_status_t Pool::create_mcx_pool(mcx_pool_t** pool) {
    if (g_pool.pool != NULL) {
        *pool = &g_pool;

        return PJ_SUCCESS;
    }
    
    pj_status_t status = pj_init();

    if (status == PJ_SUCCESS)
        status = pjlib_util_init();

    if (status == PJ_SUCCESS) {
        pj_caching_pool_init(&g_pool.pool_factory, &pj_pool_factory_default_policy, 0);
        g_pool.pool_factory_initialized = true;
    }

    if (status == PJ_SUCCESS)
        g_pool.pool = pj_pool_create(&g_pool.pool_factory.factory, "mcx_global_pool", 4096, 1024, NULL);

    if (g_pool.pool == NULL)
        status = PJ_ENOMEM;

    if (status != PJ_SUCCESS && g_pool.pool) {
        pj_pool_release(g_pool.pool);
        g_pool.pool = nullptr;
     
        if (g_pool.pool_factory_initialized) {
            pj_caching_pool_destroy(&g_pool.pool_factory);
            g_pool.pool_factory_initialized = false;
        }
    }

    if (status == PJ_SUCCESS)
        *pool = &g_pool;
    
    return status;
}

pj_status_t Pool::destroy_mcx_pool() {
    if (g_pool.pool) {
        pj_pool_release(g_pool.pool);
    }

    g_pool.pool = nullptr;

    if (g_pool.pool_factory_initialized) {
        pj_caching_pool_destroy(&g_pool.pool_factory);
        g_pool.pool_factory_initialized = false;
    }

    return PJ_SUCCESS;
}
