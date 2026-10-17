#pragma once

#include "idms/config.hpp"
#include "helper_tool/logger_helper.hpp"
#include "mcx/config.hpp"
#include "sip/config.hpp"

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
         * Gets the IDMS configuration
         * @return configuration for IDMS
         */
        const idms::Config& getIdmsConfig() const;

        /** Returns the SIP configuration. */
        const sip::Config& getSipConfig() const;

        /**
         * Gets MCX configuration
         * @return configuration for MCX
         */
        const mcx::Config& getMcxConfig() const;

        /**
         * Loads the IDMS, SIP, and MCX configurations from an INI file.
         * @param path Path to the INI file containing the configuration sections.
         * @throws std::runtime_error If the file cannot be read or the required section is missing.
         * @throws std::invalid_argument If a configuration value is invalid.
         */
        void loadConfigFromIniFile(const std::string& path);

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