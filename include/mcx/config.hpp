#pragma once

#include <string>
#include <map>
#include "sip/config.hpp"

/**
 * @file config.hpp
 * @brief Configuration model for the MCX client.
 *
 * Config stores the settings required by the client to communicate with authorization services, SIP/IMS infrastructure,
 * and MCX services. These settings may include identities, service URLs, transport options, security policies, media capabilities,
 * and runtime limits.
 *
 * Config only stores and loads settings. It does not perform authorization, SIP registration, or network communication; Client and
 * its internal services use the configured values to perform those operations.
 */
namespace mcx {
    /**
     * Provides configuration for an MCX client instance.
     *
     * A Client receives a Config object when it is constructed. Configuration values are kept separate from the protocol
     * components so that the same client API can be used with different MCX server profiles.
     */
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
         * Load configuration settings from a file or other source. This method allows the client to customize its
         * behavior based on external configuration.
         */
        bool load(const std::string& filename);

        /**
         * Save configuration settings to a file or other destination. This method allows the client to persist its
         * configuration for future use.
         */
        bool save(const std::string& filename);            
    };

    /**
     * Provides configuration for an MCX client instance that includes SIP/IMS settings.
     */
    struct Mcx : public Config {
        /**
         * The MCX ID is a unique identifier for the MCX client instance. It is used to authenticate the client with the MCX
         * server and to manage its session.
         */
        std::string id;

        /**
         * The client ID is a unique identifier for the MCX client instance. It is used to authenticate the client with the MCX
         * server and to manage its session.
         */
        std::string clientId;

        /**
         * The access token is a temporary credential used to access MCX services. It is typically obtained through an authorization 
         * process and has a limited lifetime. The token should be kept confidential and not shared with unauthorized parties.
         */
        std::string accessToken;

        /**
         * The refresh token is a credential used to obtain a new access token when the current access token expires. It allows the 
         * client to maintain access to MCX services without requiring the user to re-authenticate. The refresh token should be kept 
         * confidential and not shared with unauthorized parties.
         */
        std::string refreshToken;

        /**
         * The client secret is a confidential value used in conjunction with the client ID to authenticate the MCX client with the
         * MCX server. It should be kept secure and not shared with unauthorized parties. The client secret is typically provided by
         * the MCX server during the registration process and is used to verify the identity of the client when requesting access 
         * tokens or performing other secure operations.
         */
        std::string clientSecret;

        /**
         * The expiration timestamp indicates the time at which the current access token will expire. It is typically represented
         * as a Unix timestamp (the number of seconds since January 1, 1970). The client should monitor this value and request a
         * new access token using the refresh token before the current token expires to ensure uninterrupted access to MCX 
         * services.
         */ 
        int expireTimestamp;

        /**
         * The scope defines the permissions and access levels granted to the MCX client. It specifies the resources and actions
         * that the client is authorized to perform within the MCX system. The scope is typically defined during the authorization
         * process and may include specific capabilities such as voice communication, messaging, or group management. The client 
         * should adhere to the defined scope to ensure compliance with security policies and access control mechanisms.
         */
        std::string scope;
    };
}