#pragma once

#include <string>

/**
 * @file config.hpp
 * @brief Configuration model for the SIP client.
 * 
 * Config stores the settings required by the client to communicate with authorization services, SIP/IMS infrastructure,
 * and MCX services. These settings may include identities, service URLs, transport options, security policies, media capabilities,
 * and runtime limits.
 */
namespace sip {
    /**
     * Enumeration representing the transport protocols supported by the SIP client.
     */
    enum Protocol {
        UDP,
        TCP,
        TLS
    };

    struct Config {
        /**
         * Constructor for the Config class. Initializes configuration settings with default values.
         */
        Config();

        /**
         * Destructor for the Config class. Cleans up any resources used by the configuration settings.
         */
        ~Config();
        
        /**
         * The IMPI (IP Multimedia Private Identity) is a unique identifier used in IMS (IP Multimedia Subsystem) networks
         * to identify a user. It is typically used for authentication and authorization purposes.
         */
        std::string impiUsername;

        /**
         * The IMPU (IP Multimedia Public Identity) is a unique identifier used in IMS (IP Multimedia Subsystem) networks
         * to identify a user. It is typically used for registration and discovery purposes.
         */
        std::string impuUsername;

        /**
         * The password associated with the IMPI is used for authentication in IMS networks. It should be kept confidential and
         * not shared with unauthorized parties.
         */
        std::string password;

        /**
         * The transport protocol used for communication with the SIP/IMS infrastructure. This may include UDP, TCP, or TLS.
         */
        Protocol transportProtocol;
    };
}