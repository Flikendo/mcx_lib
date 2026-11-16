#include "api/client_lifecycle.h"

#include <iostream>
#include <stdexcept>

/**
 * Checks client creation and null-safe, repeatable start/stop lifecycle calls.
 * The test initializes a client handle, starts and stops the client multiple times, and finally deinitializes the client.
 * It also verifies that the API correctly handles null client handles by returning appropriate error codes.
 */
int main() {
    try {
        if (deinit(nullptr) != STATUS_CODE_ERR_NULL ||
            startClient(nullptr) != STATUS_CODE_ERR_NULL ||
            stopClient(nullptr) != STATUS_CODE_ERR_NULL) {
            throw std::runtime_error("Lifecycle API should reject null handles");
        }

        client_handle_t* client = init();
        if (client == nullptr) {
            throw std::runtime_error("Client initialization failed");
        }

        if (startClient(client) != STATUS_CODE_OK ||
            startClient(client) != STATUS_CODE_OK ||
            stopClient(client) != STATUS_CODE_OK ||
            stopClient(client) != STATUS_CODE_OK ||
            deinit(client) != STATUS_CODE_OK) {
            throw std::runtime_error("Client lifecycle operation failed");
        }

        std::cout << "Client lifecycle tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Client lifecycle test failed: " << error.what() << '\n';
        return 1;
    }
}
