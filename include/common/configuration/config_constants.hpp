#pragma once

// TESTING
constexpr const char* INI_FILE_PATH = "config.ini";

/**
 * INI Section Names
 */ 
constexpr const char* NETWORK_SECTION_NAME = "NETWORK";
constexpr const char* SECURITY_SECTION_NAME = "SECURITY";
constexpr const char* IDENTITY_SECTION_NAME = "IDENTITY";
constexpr const char* AUDIO_SECTION_NAME = "AUDIO";
constexpr const char* LOG_SECTION_NAME = "LOG"; 
constexpr const char* MANAGEMENT_SECTION_NAME = "MANAGEMENT";

/**
 * IDMS Key Names
 */
constexpr const char* OUTBOUND_PROXY_KEY = "outboundProxyAddress";
constexpr const char* IDMS_USERNAME_KEY = "idms_usernameForAuth";
constexpr const char* IDMS_PASSWORD_KEY = "idms_passwordForAuth";
constexpr const char* IDMS_ISSUER_KEY = "idmc_idmsIssuer";
constexpr const char* IDMS_AUTHORIZATION_ENDPOINT_KEY = "idmc_idmsAuthorizationEndpoint";
constexpr const char* IDMS_TOKEN_ENDPOINT_KEY = "idmc_idmsTokenEndpoint";
constexpr const char* IDMS_REFRESH_TOKEN_ENDPOINT_KEY = "idmc_idmsRefreshTokenEndpoint";
constexpr const char* IDMS_REDIRECT_URI_KEY = "idmc_redirectUri";

/**
 * SIP Key Names
 */
constexpr const char* SIP_IMPI_USERNAME_KEY = "sip_impiUsername";
constexpr const char* SIP_IMPU_USERNAME_KEY = "sip_impuUsername";
constexpr const char* SIP_PASSWORD_KEY = "sip_password";
constexpr const char* SIP_TRANSPORT_PROTOCOL_KEY = "sip_transportProtocol";

/**
 * MCX Key Names
 */
constexpr const char* MCPTT_ID_KEY = "mcptt_id";
constexpr const char* MCPTT_CLIENT_ID_KEY = "mcptt_clientId";
constexpr const char* MCPTT_ACCESS_TOKEN_KEY = "mcptt_accessToken";
constexpr const char* MCPTT_REFRESH_TOKEN_KEY = "mcptt_refreshToken";
constexpr const char* MCPTT_CLIENT_SECRET_KEY = "mcptt_clientSecret";
constexpr const char* MCPTT_EXPIRATION_KEY = "mcptt_expireTimestamp";
constexpr const char* MCPTT_SCOPE_KEY = "mcptt_scope";

/**
 * LOG Key Names
 */
constexpr const char* MINIMUN_CONFIG_LOG = "minimum_config_log";