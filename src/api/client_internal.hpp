#ifndef MCX_API_CLIENT_INTERNAL_HPP
#define MCX_API_CLIENT_INTERNAL_HPP

#include "common/configuration/config_manager.hpp"
#include "helper_tool/log_helper.hpp"

struct client_handler {
    ConfigManager configManager;
    Logger logger;
};

#endif
