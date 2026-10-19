#include "mcx/client.hpp"

namespace mcx {
    class Client::Impl {
        public:
            bool running = false;
    };

    Client::Client() : impl(std::make_unique<Impl>()) {}

    Client::~Client() = default;

    void Client::start() {
        impl->running = true;
    }

    void Client::stop() noexcept {
        impl->running = false;
    }

    bool Client::isRunning() {
        return impl->running;
    }
}
