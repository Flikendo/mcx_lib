#include <idms/config.hpp>
#include <mcx/config.hpp>
#include <sip/config.hpp>
#include <ini_helper.hpp>

#include <iostream>
#include <string>

int main() {
    const auto iniData = helper_tool::IniHelper::read("config.ini");
    if (iniData.empty()) {
        std::cerr << "Failed to read INI file or it was empty\n";
        return 1;
    }

    // IDMS section validation
    idms::Config config;
    config.populateFromIniFile("config.ini", "IDENTITY");

    // SIP section validation
    sip::Config sipConfig;
    sipConfig.populateFromIniFile("config.ini", "SIP");

    // MCX section validation
    mcx::Config mcxConfig;
    mcxConfig.populateFromIniFile("config.ini", "MCX");

    std::cout << "config.ini parsing and config population passed\n";
    std::cout << "Config values:\n" << config.toString() << std::endl;
    return 0;
}