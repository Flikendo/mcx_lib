#ifndef MCX_API_CLIENT_INTERNAL_HPP
#define MCX_API_CLIENT_INTERNAL_HPP

#include "common/configuration/config_manager.hpp"
#include "helper_tool/log_helper.hpp"
#include "sip/client_impl.hpp"

#include <memory>

/**
 * Represents the client handle structure used for managing the lifecycle of a SIP client.
 * It encapsulates the necessary components for configuration management, SIP communication, and logging.
 */
struct client_handle {
    // /**
    //  * The configuration manager instance responsible for loading and managing configuration settings.
    //  */
    // ConfigManager configManager;

    // /**
    //  * The SIP client implementation instance responsible for handling SIP communication.
    //  */
    // std::unique_ptr<sip::ClientImpl> sipClient;
    

    /**
     * The caching pool used for memory management within the PJSIP library.
     * It is initialized during the start process and cleaned up during the stop process.
     */
    mcx_pool_t* mcxPool;

    /**
     * Logger instance for logging configuration-related messages.
     * This can be used to log errors, warnings, or informational messages related to configuration processing.
     */
    Logger* logger;
};

/**
 * Represents the memory pool structure used for managing memory allocation within the PJSIP library.
 */
typedef struct mcx_pool {
    /**
     * The caching pool used for memory management within the PJSIP library.
     * It is initialized during the start process and cleaned up during the stop process. 
     */
    pj_caching_pool chPool;

    /**
     * The memory pool used for allocating memory within the PJSIP library.
     * It is initialized during the start process and cleaned up during the stop process.
     */
    pj_pool_t* pjPool;
} mcx_pool_t;

#endif
