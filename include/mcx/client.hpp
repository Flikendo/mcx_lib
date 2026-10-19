#pragma once

#include <memory>

namespace mcx {
    /**
     * Gateway-facing entry point for the MCX client library.
     *
     * This initial API manages the client runtime only. It does not establish
     * network connections or register an endpoint yet.
     */
    class Client {
        public:
            Client();
            ~Client();

            /**
             * Starts the client runtime. Repeated calls have no effect.
             */
            void start();

            /**
             * Stops the client runtime. Repeated calls have no effect.
             */
            void stop() noexcept;

            /**
             * Returns whether the client runtime is started.
             */
            bool isRunning();

        private:
            class Impl;
            std::unique_ptr<Impl> impl;
    };
}
