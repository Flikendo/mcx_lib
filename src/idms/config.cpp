#include "idms/config.hpp"
#include "common/configuration/config_constants.hpp"
#include "helper_tool/ini_helper.hpp"

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
        setOutboundProxyAddress(helper_tool::IniHelper::getStringValue(iniData, sectionName, OUTBOUND_PROXY_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Outbound Proxy Address: " + getOutboundProxyAddress());
        setUsernameForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_USERNAME_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Username for Auth: " + getUsernameForAuth());
        setPasswordForAuth(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_PASSWORD_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Password loaded");
        setIdmsIssuer(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_ISSUER_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Issuer: " + getIdmsIssuer());
        setIdmsAuthorizationEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_AUTHORIZATION_ENDPOINT_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Authorization Endpoint: " + getIdmsAuthorizationEndpoint());
        setIdmsTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_TOKEN_ENDPOINT_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Token Endpoint: " + getIdmsTokenEndpoint());
        setIdmsRefreshTokenEndpoint(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_REFRESH_TOKEN_ENDPOINT_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Refresh Token Endpoint: " + getIdmsRefreshTokenEndpoint());
        setIdmsRedirectUri(helper_tool::IniHelper::getStringValue(iniData, sectionName, IDMS_REDIRECT_URI_KEY));
        logger.log(LogLevel::DEBUG, "IDMS - Redirect URI: " + getIdmsRedirectUri());
    }

    std::string Config::toString() const {
        return "IdmsConfig{"
               "outboundProxyAddress='" + outboundProxyAddress + '\'' +
               ", usernameForAuth='" + usernameForAuth + '\'' +
               ", passwordForAuth='<redacted>'" +
               ", idmsIssuer='" + idmsIssuer + '\'' +
               ", idmsAuthorizationEndpoint='" + idmsAuthorizationEndpoint + '\'' +
               ", idmsTokenEndpoint='" + idmsTokenEndpoint + '\'' +
               ", idmsRefreshTokenEndpoint='" + idmsRefreshTokenEndpoint + '\'' +
               ", idmsRedirectUri='" + idmsRedirectUri + '\'' +
               '}';
    }
}