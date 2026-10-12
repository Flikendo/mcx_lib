#include "mcx/config.hpp"

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

    void Config::populateFromIniData(const helper_tool::DefHelper::ConfigMap& iniData, const std::string& sectionName) {
        setId(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_id"));
        setClientId(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_clientId"));
        setAccessToken(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_accessToken"));
        setRefreshToken(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_refreshToken"));
        setClientSecret(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_clientSecret"));
        setExpireTimestamp(helper_tool::IniHelper::getValue<std::int64_t>(iniData, sectionName, "mcx_expireTimestamp"));
        setScope(helper_tool::IniHelper::getStringValue(iniData, sectionName, "mcx_scope"));
    }

    std::string Config::toString() const {
        return "Config{"
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