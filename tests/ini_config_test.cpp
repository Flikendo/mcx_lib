#include "idms/config.hpp"
#include "mcx/config.hpp"
#include "sip/config.hpp"
#include "helper_tool/ini_helper.hpp"
#include "helper_tool/logger_helper.hpp"
#include "common/configuration/config_constants.hpp"
#include "common/configuration/config_manager.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument != "--log-level" || i + 1 >= argc) {
                std::cerr << "Usage: " << argv[0]
                          << " [--log-level DEBUG|INFO|WARNING|ERROR|CRITICAL]\n";

                return 2;
            }
            Logger::setMinimumLogLevel(Logger::parseLogLevel(argv[++i]));
        }
    } catch (const std::invalid_argument& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }

    const auto iniData = helper_tool::IniHelper::read(INI_FILE_PATH);
    if (iniData.empty()) {
        std::cerr << "Failed to read INI file or it was empty\n";
        return 1;
    }

    // IDENTITY section validation
    ConfigManager configManager;
    configManager.loadConfigFromIniFile(INI_FILE_PATH);

    std::cout << configManager.getIdmsConfig().toString() << configManager.getSipConfig().toString() <<
     configManager.getMcxConfig().toString() << std::endl;

    return 0;
}