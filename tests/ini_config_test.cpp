#include <idms/config.hpp>
#include <ini_helper.hpp>

#include <iostream>
#include <string>

int main() {
    const auto iniData = helper_tool::IniHelper::read("config.ini");
    if (iniData.empty()) {
        std::cerr << "Failed to read INI file or it was empty\n";
        return 1;
    }

    const auto section = iniData.find("IDENTITY");
    if (section == iniData.end() ||
        section->second.find("outboundProxyAddress") == section->second.end() ||
        section->second.find("idms_usernameForAuth") == section->second.end() ||
        section->second.find("idms_passwordForAuth") == section->second.end()) {
        std::cerr << "Required IDENTITY settings are missing from the INI file\n";
        return 1;
    }

    const std::string outboundProxy = helper_tool::IniHelper::getStringValue(
        iniData, "IDENTITY", "outboundProxyAddress");
    const std::string username = helper_tool::IniHelper::getStringValue(
        iniData, "IDENTITY", "idms_usernameForAuth");
    const std::string password = helper_tool::IniHelper::getStringValue(
        iniData, "IDENTITY", "idms_passwordForAuth");

    idms::Config config;
    config.setOutboundProxyAddress(outboundProxy);
    config.setUsernameForAuth(username);
    config.setPasswordForAuth(password);

    if (config.getOutboundProxyAddress() != outboundProxy ||
        config.getUsernameForAuth() != username ||
        config.getPasswordForAuth() != password) {
        std::cerr << "Parsed values did not reach the config object correctly\n";
        return 1;
    }

    std::cout << "config.ini parsing and config population passed\n";
    std::cout << "Config values:\n" << config.toString() << std::endl;
    return 0;
}