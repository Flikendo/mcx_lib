#pragma once

#include "common/configuration/config_base.hpp"
#include "helper_tool/def_helper.hpp"

#include <cstdint>
#include <string>

/**
 * @file config.hpp
 * @brief Configuration model for the MCX client.
 * 
 * Config stores the settings required by the client to communicate with authorization services, SIP/IMS infrastructure,
 * and MCX services. These settings may include identities, service URLs, transport options, security policies, media 
 * capabilities, and runtime limits.
 */
namespace mcx {
    class Config : public ConfigBase {
        public:
            /**
             * Sets the MCX IP server address.
             * @param address The MCX IP server address as a string.
             */
            void setId(const std::string& id);

            /**
             * Gets the MCX IP server address.
             * @return The MCX IP server address as a string.
             */
            std::string getId() const;

            /**
             * Sets the client ID for the MCX client.
             * @param clientId The client ID as a string.
             */
            void setClientId(const std::string& clientId);

            /**
             * Gets the client ID for the MCX client.
             * @return The client ID as a string.
             */
            std::string getClientId() const;

            /**
             * Sets the access token for the MCX client.
             * @param accessToken The access token as a string.
             */
            void setAccessToken(const std::string& accessToken);

            /**
             * Gets the access token for the MCX client.
             * @return The access token as a string.
             */
            std::string getAccessToken() const;

            /**
             * Sets the refresh token for the MCX client.
             * @param refreshToken The refresh token as a string.
             */
            void setRefreshToken(const std::string& refreshToken);

            /**
             * Gets the refresh token for the MCX client.
             * @return The refresh token as a string.
             */
            std::string getRefreshToken() const;

            /**
             * Sets the client secret for the MCX client.
             * @param clientSecret The client secret as a string.
             */
            void setClientSecret(const std::string& clientSecret);

            /**
             * Gets the client secret for the MCX client.
             * @return The client secret as a string.
             */
            std::string getClientSecret() const;

            /**
             * Sets the expiration timestamp for the access token.
             * @param expireTimestamp The expiration timestamp as an integer (Unix timestamp).
             */
            void setExpireTimestamp(std::int64_t expireTimestamp);

            /**
             * Gets the expiration timestamp for the access token.
             * @return The expiration timestamp as an integer (Unix timestamp).
             */
            std::int64_t getExpireTimestamp() const;

            /**
             * Sets the scope for the MCX client.
             * @param scope The scope as a string.
             */
            void setScope(const std::string& scope);

            /**
             * Gets the scope for the MCX client.
             * @return The scope as a string.
             */
            std::string getScope() const;

            /**
             * Returns a string representation of the configuration.
             * @return A string containing the configuration values.
             */
            std::string toString() const override;

        protected:
            /**
             * Populates the configuration settings from the provided INI data.
             * @param iniData The nested map containing the INI data.
             * @param sectionName The name of the section in the INI data to read.
             */
            void populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) override;
            
        private:
            /**
             * The MCX ID is a unique identifier for the MCX client instance. It is used to authenticate the client with the MCX
             * server and to manage its session. E.G. "sip:70501@javlaxy.com"
             */
            std::string id;

            /**
             * The client ID is a unique identifier for the MCX client instance. It is used to authenticate the client with the MCX
             * server and to manage its session. E.G. "urn:gsma:imei:10115412-000048-0"
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
            std::int64_t expireTimestamp = 0;

            /**
             * The scope defines the permissions and access levels granted to the MCX client. It specifies the resources and actions
             * that the client is authorized to perform within the MCX system. The scope is typically defined during the authorization
             * process and may include specific capabilities such as voice communication, messaging, or group management. The client 
             * should adhere to the defined scope to ensure compliance with security policies and access control mechanisms.
             */
            std::string scope;
    };
}