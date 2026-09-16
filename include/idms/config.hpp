#pragma once

#include <string>

/**
 * @file config.hpp
 * @brief Configuration model for the IDMS client.
 * 
 * Config stores the settings required by the client to communicate with authorization services, IDMS infrastructure,
 * and MCX services. These settings may include identities, service URLs, transport options, security policies, media capabilities, and runtime limits.
 */
namespace idms {
    struct Config {
        /**
         * The username for authentication with the IDMS server. This is typically used in conjunction with a password to authenticate the client.
         */
        std::string usernameForAuth;

        /**
         * The password for authentication with the IDMS server. This is typically used in conjunction with a username to authenticate the client.
         */
        std::string passwordForAuth;

        /**
         * MCX IP server address. This is the IP address of the MCX server that the client will communicate with for media control and signaling.
         */
        std::string outboundProxyAddresses;

        /**
         * The IDMS issuer is the unique identifier of the IDMS server that issues tokens for authentication and authorization. The client uses this value to validate tokens and ensure they are issued by a trusted source.
         */
        std::string idmc_idmsIssuer;

        /**
         * The IDMS authorization endpoint is the URL of the IDMS server's authorization service. The client uses this endpoint to obtain access tokens and perform other authorization-related operations.
         */
        std::string idmc_idmsAuthorizationEndpoint;

        /**
         * The IDMS token endpoint is the URL of the IDMS server's token service. The client uses this endpoint to exchange authorization codes for access tokens and refresh tokens, enabling it to access protected resources on behalf of the user.
         */
        std::string idmc_idmsTokenEndpoint;

        /**
         * The IDMS refresh token endpoint is the URL of the IDMS server's refresh token service. The client uses this endpoint to obtain new access tokens using a valid refresh token, allowing it to maintain access to protected resources without requiring the user to re-authenticate.
         */
        std::string idmc_idmsRefreshTokenEndpoint;

        /**
         * The IDMS redirect URI is the URL to which the IDMS server will redirect the user after they have authenticated and authorized the client. The client must register this URI with the IDMS server to ensure that it can receive authorization codes and access tokens securely.
         */
        std::string idmc_redirectUri;
    };
}