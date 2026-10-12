#include <ini_helper.hpp>
#include <idms/config.hpp>

namespace idms {
    void Config::setOutboundProxyAddress(const std::string& outboundProxyAddress) {
        this->outboundProxyAddress = outboundProxyAddress;
    }

    std::string Config::getOutboundProxyAddress() const {
        return outboundProxyAddress;
    }

    void Config::setUsernameForAuth(const std::string& usernameForAuth) {
        this->usernameForAuth = usernameForAuth;
    }

    std::string Config::getUsernameForAuth() const {
        return usernameForAuth;
    }

    void Config::setPasswordForAuth(const std::string& passwordForAuth) {
        this->passwordForAuth = passwordForAuth;
    }

    std::string Config::getPasswordForAuth() const {
        return passwordForAuth;
    }

    void Config::setIdmsIssuer(const std::string& issuer) {
        this->idmsIssuer = issuer;
    }

    std::string Config::getIdmsIssuer() const {
        return idmsIssuer;
    }

    void Config::setIdmsAuthorizationEndpoint(const std::string& idmsAuthorizationEndpoint) {
        this->idmsAuthorizationEndpoint = idmsAuthorizationEndpoint;
    }

    std::string Config::getIdmsAuthorizationEndpoint() const {
        return idmsAuthorizationEndpoint;
    }

    void Config::setIdmsTokenEndpoint(const std::string& endpoint) {
        this->idmsTokenEndpoint = endpoint;
    }

    std::string Config::getIdmsTokenEndpoint() const {
        return idmsTokenEndpoint;
    }

    void Config::setIdmsRefreshTokenEndpoint(const std::string& idmsRefreshTokenEndpoint) {
        this->idmsRefreshTokenEndpoint = idmsRefreshTokenEndpoint;
    }

    std::string Config::getIdmsRefreshTokenEndpoint() const {
        return idmsRefreshTokenEndpoint;
    }

    void Config::setIdmsRedirectUri(const std::string& uri) {
        this->idmsRedirectUri = uri;
    }

    std::string Config::getIdmsRedirectUri() const {
        return idmsRedirectUri;
    }

    void Config::populateFromIniData(const helper_tool::DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setOutboundProxyAddress(helper_tool::IniHelper::getStringValue(iniData, sectionName, "outboundProxyAddress"));
        setUsernameForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_usernameForAuth"));
        setPasswordForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_passwordForAuth"));
        setIdmsIssuer(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_issuer"));
        setIdmsAuthorizationEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_authorizationEndpoint"));
        setIdmsTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_tokenEndpoint"));
        setIdmsRefreshTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_refreshTokenEndpoint"));
        setIdmsRedirectUri(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_redirectUri"));
    }

    std::string Config::toString() const {
        return "Config{"
               "outboundProxyAddress='" + outboundProxyAddress + '\'' +
               ", usernameForAuth='" + usernameForAuth + '\'' +
               ", passwordForAuth='" + passwordForAuth + '\'' +
               ", idmsIssuer='" + idmsIssuer + '\'' +
               ", idmsAuthorizationEndpoint='" + idmsAuthorizationEndpoint + '\'' +
               ", idmsTokenEndpoint='" + idmsTokenEndpoint + '\'' +
               ", idmsRefreshTokenEndpoint='" + idmsRefreshTokenEndpoint + '\'' +
               ", idmsRedirectUri='" + idmsRedirectUri + '\'' +
               '}';
    }
}