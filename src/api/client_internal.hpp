#ifndef MCX_API_CLIENT_INTERNAL_HPP
#define MCX_API_CLIENT_INTERNAL_HPP

#include "common/configuration/config_manager.hpp"
#include "helper_tool/log_helper.hpp"
#include "sip/client_impl.hpp"

#include <memory>

struct client_handle {
    /**
     * The configuration manager instance responsible for loading and managing configuration settings.
     */
    ConfigManager configManager;

    /**
     * The SIP client implementation instance responsible for handling SIP communication.
     */
    std::unique_ptr<sip::ClientImpl> sipClient;

    /**
     * Logger instance for logging configuration-related messages.
     * This can be used to log errors, warnings, or informational messages related to configuration processing.
     */
    Logger logger;
};

#endif
