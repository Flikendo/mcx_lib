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

    // IDENTITY section validation
    idms::Config idmsConfig;
    idmsConfig.populateFromIniFile("config.ini", "IDENTITY");

    sip::Config sipConfig;
    sipConfig.populateFromIniFile("config.ini", "IDENTITY");

    mcx::Config mcxConfig;
    mcxConfig.populateFromIniFile("config.ini", "IDENTITY");

    std::cout << idmsConfig.toString() << sipConfig.toString() << mcxConfig.toString() << std::endl;

    return 0;
}