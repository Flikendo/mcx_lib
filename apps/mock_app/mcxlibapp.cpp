#include "api/client.h"
#include "common/configuration/config_constants.hpp"

#include <iostream>

void printUsage() {
    std::cout << "Usage: mock_app [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help      Show this help message\n";
    std::cout << "  -s, --start     Start the MCX client\n";
    std::cout << "  -q, --quit      Stop the MCX client and quit\n";
    std::cout << "Enter command: \n";
}

int main() {
    client_handler_t* client = nullptr;
    char command = '\0';

    std::cout << "MCX library client mock\n";

    while (command != 'q') {
        printUsage();

        if (!(std::cin >> command)) {
            if (client != nullptr) {
                deinit(client);
            }
            return 0;
        }

        switch (command) {
            case 's':
                client = init();
                loadIniFile(client, INI_FILE_PATH, true);
                std::cout << "Client running\n";
                break;
            case 'q':
                if (client != nullptr) {
                    deinit(client);
                }
                std::cout << "Application stopped\n";
                break;
            default:
                std::cout << "Unknown command\n";
                break;
        }
    }

    return 0;
}