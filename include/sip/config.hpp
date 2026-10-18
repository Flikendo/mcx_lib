#pragma once

#include "common/configuration/config_base.hpp"
#include "common/network/transport_protocol.hpp"
#include "helper_tool/def_helper.hpp"
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
    class Config : public ConfigBase {
        public:
            /**
             * Sets the IMPI (IP Multimedia Private Identity) username.
             * @param username The IMPI username as a string.
             */
            void setImpiUsername(const std::string& impiUsername);

            /**
             * Gets the IMPI (IP Multimedia Private Identity) username.
             * @return The IMPI username as a string.
             */
            std::string getImpiUsername() const;

            /**
             * Sets the IMPU (IP Multimedia Public Identity) username.
             * @param username The IMPU username as a string.
             */
            void setImpuUsername(const std::string& impuUsername);

            /**
             * Gets the IMPU (IP Multimedia Public Identity) username.
             * @return The IMPU username as a string.
             */
            std::string getImpuUsername() const;

            /**
             * Sets the password associated with the IMPI.
             * @param password The password as a string.
             */
            void setPassword(const std::string& password);

            /**
             * Gets the password associated with the IMPI.
             * @return The password as a string.
             */
            std::string getPassword() const;

            /**
             * Sets the transport protocol used for communication with the SIP/IMS infrastructure.
             * @param protocol The transport protocol (UDP, TCP, or TLS).
             */
            void setTransportProtocol(Protocol protocol);

            /**
             * Gets the transport protocol used for communication with the SIP/IMS infrastructure.
             * @return The transport protocol (UDP, TCP, or TLS).
             */
            Protocol getTransportProtocol() const;
        
            /**
             * Returns a string representation of the configuration values.
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
            Protocol transportProtocol = Protocol::UDP;
    };
}