#pragma once

#include <string>
#include <common/iconfig.hpp>

/**
 * @file config.hpp
 * @brief Configuration model for the IDMS client.
 * 
 * Config stores the settings required by the client to communicate with authorization services, IDMS infrastructure,
 * and MCX services. These settings may include identities, service URLs, transport options, security policies, media capabilities, and runtime limits.
 */
namespace idms {
    struct Config : public common::IConfig {
        public:
            /**
             * Constructor for the Config class. Initializes configuration settings with default values.
             */
            Config() = default;

            /**
             * Sets the outbound proxy address for the MCX server.
             * @param address The outbound proxy address as a string.
             */
            void setOutboundProxyAddress(const std::string& outboundProxyAddress);

            /**
             * Gets the outbound proxy address for the MCX server.
             * @return The outbound proxy address as a string.
             */
            std::string getOutboundProxyAddress() const;

            /**
             * Sets the username for authentication with the IDMS server.
             * @param username The username as a string.
             */
            void setUsernameForAuth(const std::string& usernameForAuth);

            /**
             * Gets the username for authentication with the IDMS server.
             * @return The username as a string.
             */
            std::string getUsernameForAuth() const;

            /**
             * Sets the password for authentication with the IDMS server.
             * @param password The password as a string.
             */
            void setPasswordForAuth(const std::string& passwordForAuth);

            /**
             * Gets the password for authentication with the IDMS server.
             * @return The password as a string.
             */
            std::string getPasswordForAuth() const;

            /**
             * Sets the IDMS issuer URL.
             * @param issuer The IDMS issuer URL as a string.
             */
            void setIdmsIssuer(const std::string& idmsIssuer);

            /**
             * Gets the IDMS issuer URL.
             * @return The IDMS issuer URL as a string.
             */
            std::string getIdmsIssuer() const;

            /**
             * Sets the IDMS authorization endpoint URL.
             * @param endpoint The IDMS authorization endpoint URL as a string.
             */
            void setIdmsAuthorizationEndpoint(const std::string& idmsAuthorizationEndpoint);

            /**
             * Gets the IDMS authorization endpoint URL.
             * @return The IDMS authorization endpoint URL as a string.
             */
            std::string getIdmsAuthorizationEndpoint() const;

            /**
             * Sets the IDMS token endpoint URL.
             * @param endpoint The IDMS token endpoint URL as a string.
             */
            void setIdmsTokenEndpoint(const std::string& idmsTokenEndpoint);

            /**
             * Gets the IDMS token endpoint URL.
             * @return The IDMS token endpoint URL as a string.
             */
            std::string getIdmsTokenEndpoint() const;

            /**
             * Sets the IDMS refresh token endpoint URL.
             * @param endpoint The IDMS refresh token endpoint URL as a string.
             */
            void setIdmsRefreshTokenEndpoint(const std::string& idmsRefreshTokenEndpoint);

            /**
             * Gets the IDMS refresh token endpoint URL.
             * @return The IDMS refresh token endpoint URL as a string.
             */
            std::string getIdmsRefreshTokenEndpoint() const;

            /**
             * Sets the IDMS redirect URI.
             * @param uri The IDMS redirect URI as a string.
             */
            void setIdmsRedirectUri(const std::string& idmsRedirectUri);

            /**
             * Gets the IDMS redirect URI.
             * @return The IDMS redirect URI as a string.
             */
            std::string getIdmsRedirectUri() const;

            /**
             * Returns a string representation of the configuration settings.
             * @return A string containing the configuration settings in a human-readable format.
             */
            std::string toString() const override;

        private:
            /**
             * MCX IP server address. This is the IP address of the MCX server that the client will communicate with for media control and signaling.
             */
            std::string outboundProxyAddress;

            /**
             * The username for authentication with the IDMS server. This is typically used in conjunction with a password to authenticate the client.
             */
            std::string usernameForAuth;

            /**
             * The password for authentication with the IDMS server. This is typically used in conjunction with a username to authenticate the client.
             */
            std::string passwordForAuth;

            /**
             * The IDMS issuer is the unique identifier of the IDMS server that issues tokens for authentication and authorization. The client uses this value to validate tokens and ensure they are issued by a trusted source.
             */
            std::string idmsIssuer;

            /**
             * The IDMS authorization endpoint is the URL of the IDMS server's authorization service. The client uses this endpoint to obtain access tokens and perform other authorization-related operations.
             */
            std::string idmsAuthorizationEndpoint;

            /**
             * The IDMS token endpoint is the URL of the IDMS server's token service. The client uses this endpoint to exchange authorization codes for access tokens and refresh tokens, enabling it to access protected resources on behalf of the user.
             */
            std::string idmsTokenEndpoint;

            /**
             * The IDMS refresh token endpoint is the URL of the IDMS server's refresh token service. The client uses this endpoint to obtain new access tokens using a valid refresh token, allowing it to maintain access to protected resources without requiring the user to re-authenticate.
             */
            std::string idmsRefreshTokenEndpoint;

            /**
             * The IDMS redirect URI is the URL to which the IDMS server will redirect the user after they have authenticated and authorized the client. The client must register this URI with the IDMS server to ensure that it can receive authorization codes and access tokens securely.
             */
            std::string idmsRedirectUri;
    };
}