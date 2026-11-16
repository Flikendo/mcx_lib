#ifndef MCX_API_CLIENT_INTERNAL_HPP
#define MCX_API_CLIENT_INTERNAL_HPP

#include "common/configuration/config_manager.hpp"
#include "helper_tool/log_helper.hpp"
#include "sip/client_impl.hpp"

#include <memory>

struct client_handle {
    ConfigManager configManager;
    std::unique_ptr<sip::ClientImpl> sipClient;
    Logger logger;
};

#endif
