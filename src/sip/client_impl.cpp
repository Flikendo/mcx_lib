#include "sip/client_impl.hpp"

namespace sip {
    ClientImpl::ClientImpl() = default;

    ClientImpl::~ClientImpl() {
        stop();
    }

    pj_status_t ClientImpl::start() {
        if (endpoint != nullptr) {
            return PJ_SUCCESS;
        }

        pj_status_t status = pj_init();

        if (status != PJ_SUCCESS) {
            return status;
        }
        pjInitialized = true;

        pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);
        poolInitialized = true;

        status = pjsip_endpt_create(&cp.factory, "", &endpoint);
        if (status != PJ_SUCCESS) {
            stop();
            return status;
        }

        return PJ_SUCCESS;
    }

    pj_status_t ClientImpl::stop() {
        if (endpoint != nullptr) {
            pjsip_endpt_destroy(endpoint);
            endpoint = nullptr;
        }

        if (poolInitialized) {
            pj_caching_pool_destroy(&cp);
            poolInitialized = false;
        }

        if (pjInitialized) {
            pjInitialized = false;
            pj_shutdown();
        }

        return PJ_SUCCESS;
    }
}