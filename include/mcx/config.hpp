/*
* Configuration settings for the client.
*/
#pragma once

namespace mcx {
    class Config {
        public:
            /*
            * Constructor for the Config class. Initializes configuration settings with default values.
            */
            Config();

            /*
            * Destructor for the Config class. Cleans up any resources used by the configuration settings.
            */
            ~Config();

            /*
            * Load configuration settings from a file or other source. This method allows the client to customize its behavior based on external configuration.
            */
            bool load(const std::string& filename);

            /*
            * Save configuration settings to a file or other destination. This method allows the client to persist its configuration for future use.
            */
            bool save(const std::string& filename);

        private:
            /*
            * Internal representation of configuration settings. This member variable holds various options that can be used to customize the behavior of the client.
            */
            std::map<std::string, std::string> settings;
    };
}