#pragma once

#include "mcx/config.hpp"

/*
 * The Client class is responsible for managing the lifecycle of the client application. It provides methods to start and stop the client, ensuring that resources are properly initialized and cleaned up.
 */

namespace mcx {
    class Client {
        public:
            /*
            * Constructor for the Client class. Initializes the client with the provided configuration and prepares it for use.
            */
            Client(mcx::Config config);
            
            /**
            * Destructor for the Client class. Cleans up resources and ensures proper shutdown.
            */
            ~Client();

            /*
            * Start the client and initialize necessary resources. This method should be called before using the client to ensure it is properly set up.
            */
            bool start();

            /*
            * Stop the client and clean up resources. This method should be called when the client is no longer needed to ensure proper resource management.
            */
            bool stop();

        private:
            /*
            * Configuration settings for the client. This member variable holds various configuration options that can be used to customize the behavior of the client.
            */  
            Config config;
    };
}