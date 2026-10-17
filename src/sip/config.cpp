#include "sip/config.hpp"
#include "helper_tool/ini_helper.hpp"
#include "common/configuration/config_constants.hpp"

namespace sip {
    void Config::setImpiUsername(const std::string& impiUsername) {
        this->impiUsername = impiUsername;
    }

    std::string Config::getImpiUsername() const {
        return this->impiUsername;
    }

    void Config::setImpuUsername(const std::string& impuUsername) {
        this->impuUsername = impuUsername;
    }

    std::string Config::getImpuUsername() const {
        return this->impuUsername;
    }

    void Config::setPassword(const std::string& password) {
        this->password = password;
    }

    std::string Config::getPassword() const {
        return this->password;
    }

    void Config::setTransportProtocol(Protocol protocol) {
        this->transportProtocol = protocol;
    }

    Protocol Config::getTransportProtocol() const {
        return this->transportProtocol;
    }

    void Config::populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setImpiUsername(helper_tool::IniHelper::getStringValue(iniData, sectionName, SIP_IMPI_USERNAME_KEY));
        logger.log(LogLevel::DEBUG, "SIP - IMPI Username: " + getImpiUsername());
        setImpuUsername(helper_tool::IniHelper::getStringValue(iniData, sectionName, SIP_IMPU_USERNAME_KEY));
        logger.log(LogLevel::DEBUG, "SIP - IMPU Username: " + getImpuUsername());
        setPassword(helper_tool::IniHelper::getStringValue(iniData, sectionName, SIP_PASSWORD_KEY));
        logger.log(LogLevel::DEBUG, "SIP - Password loaded");

        const auto protocolStr = helper_tool::IniHelper::getStringValue(iniData, sectionName, SIP_TRANSPORT_PROTOCOL_KEY);
        if (protocolStr == "0") {
            setTransportProtocol(Protocol::UDP);
            logger.log(LogLevel::DEBUG, "SIP - Transport Protocol: UDP");
        } else if (protocolStr == "1") {
            setTransportProtocol(Protocol::TCP);
            logger.log(LogLevel::DEBUG, "SIP - Transport Protocol: TCP");
        } else if (protocolStr == "2") {
            setTransportProtocol(Protocol::TLS);
            logger.log(LogLevel::DEBUG, "SIP - Transport Protocol: TLS");
        } else {
            logger.log(LogLevel::ERROR, "Invalid transport protocol: " + protocolStr);
        }
    }

    std::string Config::toString() const {
        return "SipConfig{"
               "impiUsername='" + impiUsername + '\'' +
               ", impuUsername='" + impuUsername + '\'' +
               ", password='<redacted>'" +
               ", transportProtocol=" + std::to_string(static_cast<int>(transportProtocol)) +
               '}';
    }
}