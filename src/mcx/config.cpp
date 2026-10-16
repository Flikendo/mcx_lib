#include "mcx/config.hpp"
#include "ini_helper.hpp"
#include "common/configuration/config_constants.hpp"

namespace mcx {
    void Config::setId(const std::string& id) {
        this->id = id;
    }

    std::string Config::getId() const {
        return this->id;
    }

    void Config::setClientId(const std::string& clientId) {
        this->clientId = clientId;
    }

    std::string Config::getClientId() const {
        return this->clientId;
    }

    void Config::setAccessToken(const std::string& accessToken) {
        this->accessToken = accessToken;
    }

    std::string Config::getAccessToken() const {
        return this->accessToken;
    }

    void Config::setRefreshToken(const std::string& refreshToken) {
        this->refreshToken = refreshToken;
    }

    std::string Config::getRefreshToken() const {
        return this->refreshToken;
    }

    void Config::setClientSecret(const std::string& clientSecret) {
        this->clientSecret = clientSecret;
    }

    std::string Config::getClientSecret() const {
        return this->clientSecret;
    }

    void Config::setExpireTimestamp(std::int64_t expireTimestamp) {
        this->expireTimestamp = expireTimestamp;
    }

    std::int64_t Config::getExpireTimestamp() const {
        return this->expireTimestamp;
    }

    void Config::setScope(const std::string& scope) {
        this->scope = scope;
    }

    std::string Config::getScope() const {
        return this->scope;
    }

    void Config::populateFromIniData(const DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setId(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_ID_KEY));
        logger.log(LogLevel::DEBUG, "MCX - ID: " + getId());
        setClientId(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_CLIENT_ID_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Client ID: " + getClientId());
        setAccessToken(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_ACCESS_TOKEN_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Access Token " + getAccessToken());
        setRefreshToken(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_REFRESH_TOKEN_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Refresh Token " + getRefreshToken());
        setClientSecret(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_CLIENT_SECRET_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Client Secret " + getClientSecret());
        setExpireTimestamp(helper_tool::IniHelper::getValue<std::int64_t>(iniData, sectionName, MCPTT_EXPIRATION_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Expire Timestamp: " + std::to_string(getExpireTimestamp()));
        setScope(helper_tool::IniHelper::getStringValue(iniData, sectionName, MCPTT_SCOPE_KEY));
        logger.log(LogLevel::DEBUG, "MCX - Scope: " + getScope());
    }

    std::string Config::toString() const {
        return "McxConfig{"
               "id='" + id + '\'' +
               ", clientId='" + clientId + '\'' +
               ", accessToken='" + accessToken + '\'' +
               ", refreshToken='" + refreshToken + '\'' +
               ", clientSecret='" + clientSecret + '\'' +
               ", expireTimestamp=" + std::to_string(expireTimestamp) +
               ", scope='" + scope + '\'' +
               '}';
    }
}