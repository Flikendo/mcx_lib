#pragma once

#include <string>

namespace common {
    struct IConfig {
        public:
            /**
             * Destructor for the Config class. Cleans up any resources used by the configuration settings.
             */
            virtual ~IConfig() = default;
            
            /**
             * Returns a string representation of the configuration settings.
             * @return A string containing the configuration settings in a human-readable format.
             */
            virtual std::string toString() const = 0;
    };
}