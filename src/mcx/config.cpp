#include "mcx/config.hpp"

#include <utility>

namespace mcx {

    Config::Config(std::string id, std::string clientId, std::string accessToken, std::string refreshToken,
    std::string clientSecret, std::int64_t expireTimestamp, std::string scope) {
        this->id = id;
        this->clientId = clientId;
        this->accessToken = accessToken;
        this->refreshToken = refreshToken;
        this->clientSecret = clientSecret;
        this->expireTimestamp = expireTimestamp;
        this->scope = scope;
    }

} // namespace mcx