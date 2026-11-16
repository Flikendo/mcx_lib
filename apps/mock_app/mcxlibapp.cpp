#include "api/client_config.h"
#include "api/client_lifecycle.h"
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
    client_handle_t* client = nullptr;
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
                if (client != nullptr) {
                    std::cout << "Client already initialized\n";
                    break;
                }

                client = init();
                if (client == nullptr) {
                    std::cerr << "Client initialization failed\n";
                    break;
                }

                if (loadIniFile(client, INI_FILE_PATH, true) != STATUS_CODE_OK) {
                    std::cerr << "Client configuration failed\n";
                    deinit(client);
                    client = nullptr;
                    break;
                }

                if (startClient(client) != STATUS_CODE_OK) {
                    std::cerr << "Client start failed\n";
                    deinit(client);
                    client = nullptr;
                    break;
                }

                std::cout << "PJSIP endpoint initialized (no transport yet)\n";
                break;
            case 'q':
                if (client != nullptr) {
                    if (stopClient(client) != STATUS_CODE_OK) {
                        std::cerr << "Client stop failed\n";
                    }
                    deinit(client);
                    client = nullptr;
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