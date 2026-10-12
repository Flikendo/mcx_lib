#include <sip/config.hpp>

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

    void Config::populateFromIniData(const helper_tool::DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setImpiUsername(helper_tool::IniHelper::getStringValue(iniData, sectionName, "impi_username"));
        setImpuUsername(helper_tool::IniHelper::getStringValue(iniData, sectionName, "impu_username"));
        setPassword(helper_tool::IniHelper::getStringValue(iniData, sectionName, "password"));

        const auto protocolStr = helper_tool::IniHelper::getStringValue(iniData, sectionName, "transport_protocol");
        if (protocolStr == "0") {
            setTransportProtocol(Protocol::UDP);
        } else if (protocolStr == "1") {
            setTransportProtocol(Protocol::TCP);
        } else if (protocolStr == "2") {
            setTransportProtocol(Protocol::TLS);
        } else {
            throw std::runtime_error("Invalid transport protocol: " + protocolStr);
        }
    }

    std::string Config::toString() const {
        return "Config{"
               "impiUsername='" + impiUsername + '\'' +
               ", impuUsername='" + impuUsername + '\'' +
               ", password='" + password + '\'' +
               ", transportProtocol=" + std::to_string(static_cast<int>(transportProtocol)) +
               '}';
    }
}