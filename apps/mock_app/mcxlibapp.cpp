#include "common/configuration/config_constants.hpp"
#include "common/configuration/config_manager.hpp"

#include <iostream>

/**
 * Output lines
 */
constexpr const char* START_LIBRARY = "*** STARTING LIBRARY MCX_LIB ***";

/**
 * Commands
 */
constexpr const char* LOAD_CONFIG_FILE = "Loading INI config file"; 
constexpr const char* LIST_COMMANDS = "*** List of commads ***";
constexpr const char* QUIT_APP_MOCK = "Quit APP mock";

void loadConfigFromFile(ConfigManager& configManager) {
    std::cout << LOAD_CONFIG_FILE << std::endl;

    configManager.loadConfigFromIniFile(INI_FILE_PATH);

    std::cout << configManager.getIdmsConfig().toString() << configManager.getSipConfig().toString() <<
    configManager.getMcxConfig().toString();
}

void listCommands() {
    std::cout << std::endl;
    std::cout << LIST_COMMANDS << std::endl;
    std::cout << "q -> " << QUIT_APP_MOCK << std::endl;
    std::cout << "s -> " << LOAD_CONFIG_FILE << std::endl;
}

int main() {
    char command = '\0';
    ConfigManager configManager;

    std::cout << START_LIBRARY << std::endl;

    while (command != 'q') {
        listCommands();
        std::cout << "Enter a command: " << std::endl;
        std::cin >> command;

        switch (command) {
            case 'q': 
                std::cout << QUIT_APP_MOCK << std:: endl;
            break;
            case 's': 
                loadConfigFromFile(configManager);
            break;
        }
    }
}