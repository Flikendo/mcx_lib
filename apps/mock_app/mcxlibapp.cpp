#include "api/client.h"
#include "common/configuration/config_constants.hpp"

#include <iostream>

int main() {
    client_handler_t* client = nullptr;
    char command = '\0';

    std::cout << "MCX library client mock\n";

    while (command != 'q') {
        std::cout << "s = start, q = stop and quit\nEnter command: ";
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
                std::cout << "Client stopped\n";
                break;
            default:
                std::cout << "Unknown command\n";
                break;
        }
    }

    return 0;
}