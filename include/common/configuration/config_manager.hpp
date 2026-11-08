#pragma once

#include "idms/config.hpp"
#include "helper_tool/log_helper.hpp"
#include "mcx/config.hpp"
#include "sip/config.hpp"
#include "api/data/types.h"

#include <string>

/**
 * @file config_manager.hpp
 * @brief Configuration manager for the MCX client.
 *
 * Owns and loads the IDMS, SIP, and MCX configuration objects.
 */
class ConfigManager {
    public:
        /**
         * Constructor
         */
        ConfigManager() = default;

        /**
         * Destructor
         */
        ~ConfigManager() = default;

        /**
         * Gets the IDMS configuration
         * @return configuration for IDMS
         */
        const idms::Config& getIdmsConfig() const;

        /** 
         * Returns the SIP configuration.
         */
        const sip::Config& getSipConfig() const;

        /**
         * Gets MCX configuration
         * @return configuration for MCX
         */
        const mcx::Config& getMcxConfig() const;

        /**
         * Loads the IDMS, SIP, and MCX configurations from an INI file.
         * @param config configuration sections.
         * @param isFromPath true if config has to be set from path file, false when it has to be set from a string.
         * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
         */
        status_code_t loadConfig(const std::string& config, bool isFromPath);

    private:
        /**
         * Configuration instance for IDMS
         */
        idms::Config idmsConfig;

        /**
         * Configuration instance for SIP
         */
        sip::Config sipConfig;

        /**
         * Configuration instance for MCX
         */
        mcx::Config mcxConfig;

        /**
         * Logger instance for logging configuration-related messages.
         * This can be used to log errors, warnings, or informational messages related to configuration processing
         */
        Logger logger;
};