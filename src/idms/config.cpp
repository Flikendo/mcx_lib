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

    void Config::populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setOutboundProxyAddress(helper_tool::IniHelper::getStringValue(iniData, sectionName, "outboundProxyAddress"));
        logger.log(LogLevel::INFO, "Outbound Proxy Address: " + getOutboundProxyAddress());
        setUsernameForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_usernameForAuth"));
        logger.log(LogLevel::INFO, "Username for Auth: " + getUsernameForAuth());
        setPasswordForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_passwordForAuth"));
        logger.log(LogLevel::INFO, "Password for Auth: " + getPasswordForAuth());
        setIdmsIssuer(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_issuer"));
        logger.log(LogLevel::INFO, "IDMS Issuer: " + getIdmsIssuer());
        setIdmsAuthorizationEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_authorizationEndpoint"));
        logger.log(LogLevel::INFO, "IDMS Authorization Endpoint: " + getIdmsAuthorizationEndpoint());
        setIdmsTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_tokenEndpoint"));
        logger.log(LogLevel::INFO, "IDMS Token Endpoint: " + getIdmsTokenEndpoint());
        setIdmsRefreshTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_refreshTokenEndpoint"));
        logger.log(LogLevel::INFO, "IDMS Refresh Token Endpoint: " + getIdmsRefreshTokenEndpoint());
        setIdmsRedirectUri(helper_tool::IniHelper::getStringValue(iniData, sectionName, "idms_redirectUri"));
        logger.log(LogLevel::INFO, "IDMS Redirect URI: " + getIdmsRedirectUri());
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