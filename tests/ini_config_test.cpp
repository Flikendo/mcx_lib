#include "common/configuration/config_constants.hpp"
#include "common/configuration/config_manager.hpp"
#include "helper_tool/ini_helper.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    void require(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    class TempIniFile {
        public:
            TempIniFile() {
                const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
                path = std::filesystem::temp_directory_path() /
                       ("mcx_lib_ini_config_test_" + std::to_string(ticks) + ".ini");
            }

            ~TempIniFile() {
                std::error_code error;
                std::filesystem::remove(path, error);
            }

            void write(const std::string& transportProtocol) const {
                std::ofstream file(path);
                if (!file) {
                    throw std::runtime_error("Could not create temporary INI test file");
                }

                file << "[IDENTITY]\n"
                     << "outboundProxyAddress=proxy.test\n"
                     << "idms_usernameForAuth=test-user\n"
                     << "idms_passwordForAuth=test-idms-password\n"
                     << "idmc_idmsIssuer=https://issuer.test\n"
                     << "idmc_idmsAuthorizationEndpoint=https://auth.test\n"
                     << "idmc_idmsTokenEndpoint=https://token.test\n"
                     << "idmc_idmsRefreshTokenEndpoint=https://refresh.test\n"
                     << "idmc_redirectUri=https://redirect.test\n"
                     << "sip_impiUsername=test-impi\n"
                     << "sip_impuUsername=test-impu\n"
                     << "sip_password=test-sip-password\n"
                     << "sip_transportProtocol=" << transportProtocol << '\n'
                     << "mcptt_id=test-mcx-id\n"
                     << "mcptt_clientId=test-client\n"
                     << "mcptt_accessToken=test-access-token\n"
                     << "mcptt_refreshToken=test-refresh-token\n"
                     << "mcptt_clientSecret=test-client-secret\n"
                     << "mcptt_expireTimestamp=123456789\n"
                     << "mcptt_scope=test-scope\n";

                if (!file) {
                    throw std::runtime_error("Could not write temporary INI test file");
                }
            }

            const std::filesystem::path& getPath() const {
                return path;
            }

        private:
            std::filesystem::path path;
    };
}

int main() {
    try {
        const DefHelper::ConfigMap data = {
            {IDENTITY_SECTION_NAME, {
                {"numericValue", "42"},
                {"invalidNumericValue", "42tail"},
                {"emptyValue", ""}
            }}
        };

        require(helper_tool::IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "numericValue") == 42,
                "Typed INI value conversion failed");
        require(helper_tool::IniHelper::getStringValue(data, IDENTITY_SECTION_NAME, "emptyValue").empty(),
                "Empty INI values should remain empty");
        require(helper_tool::IniHelper::getStringValue(data, IDENTITY_SECTION_NAME, "missingValue").empty(),
                "Missing string values should return an empty string");

        bool missingValueThrew = false;
        try {
            helper_tool::IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "missingValue");
        } catch (const std::out_of_range&) {
            missingValueThrew = true;
        }
        require(missingValueThrew, "Typed lookup should reject a missing key");

        bool invalidNumericValueThrew = false;
        try {
            helper_tool::IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "invalidNumericValue");
        } catch (const std::invalid_argument&) {
            invalidNumericValueThrew = true;
        }
        require(invalidNumericValueThrew, "Typed lookup should reject trailing non-numeric characters");

        idms::Config config;
        bool missingSectionThrew = false;
        try {
            config.loadFromIniData(data, "MISSING");
        } catch (const std::runtime_error&) {
            missingSectionThrew = true;
        }
        require(missingSectionThrew, "Configuration loading should reject a missing section");

        ConfigManager configManager;
        TempIniFile iniFile;
        iniFile.write("1");
        configManager.loadConfigFromIniFile(iniFile.getPath().string());

        require(configManager.getIdmsConfig().getUsernameForAuth() == "test-user",
                "IDMS configuration did not load");
        require(configManager.getSipConfig().getTransportProtocol() == Protocol::TCP,
                "SIP transport protocol did not load");
        require(configManager.getMcxConfig().getExpireTimestamp() == 123456789,
                "MCX expiration timestamp did not load");

        const std::string idmsOutput = configManager.getIdmsConfig().toString();
        const std::string sipOutput = configManager.getSipConfig().toString();
        const std::string mcxOutput = configManager.getMcxConfig().toString();
        require(idmsOutput.find("test-idms-password") == std::string::npos,
                "IDMS password was exposed in configuration output");
        require(sipOutput.find("test-sip-password") == std::string::npos,
                "SIP password was exposed in configuration output");
        require(mcxOutput.find("test-access-token") == std::string::npos &&
                mcxOutput.find("test-refresh-token") == std::string::npos &&
                mcxOutput.find("test-client-secret") == std::string::npos,
                "MCX credentials were exposed in configuration output");

        ConfigManager invalidConfigManager;
        iniFile.write("invalid");
        bool invalidProtocolThrew = false;
        try {
            invalidConfigManager.loadConfigFromIniFile(iniFile.getPath().string());
        } catch (const std::invalid_argument&) {
            invalidProtocolThrew = true;
        }
        require(invalidProtocolThrew, "Invalid SIP protocol should reject the configuration");

        std::cout << "INI configuration tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "INI configuration test failed: " << error.what() << '\n';
        return 1;
    }
}
