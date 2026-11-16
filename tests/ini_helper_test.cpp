#include "common/configuration/config_constants.hpp"
#include "helper_tool/ini_helper.hpp"
#include "idms/config.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    void require(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
}

/**
 * Checks INI parsing, typed lookups, and missing or invalid values.
 * The test verifies that the INI helper correctly parses configuration data, retrieves typed values,
 * and handles missing or invalid entries appropriately.
 * It also checks that the configuration loading process correctly identifies missing sections and throws exceptions as expected.
 */
int main() {
    try {
        const DefHelper::ConfigMap data = {
            {IDENTITY_SECTION_NAME, {
                {"numericValue", "42"},
                {"invalidNumericValue", "42tail"},
                {"emptyValue", ""}
            }}
        };

        const DefHelper::ConfigMap parsed = IniHelper::fromBuffer(
            "[IDENTITY]; first=one, second=two\nthird=three\r\n[LOG],minimum_config_log=INFO");
        require(parsed.at("IDENTITY").at("first") == "one" &&
                parsed.at("IDENTITY").at("second") == "two" &&
                parsed.at("IDENTITY").at("third") == "three" &&
                parsed.at("LOG").at("minimum_config_log") == "INFO",
                "INI parser should accept semicolon, comma, and newline separators");

        require(IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "numericValue") == 42,
                "Typed INI conversion failed");
        require(IniHelper::getStringValue(data, IDENTITY_SECTION_NAME, "emptyValue").empty(),
                "Empty INI value should remain empty");
        require(IniHelper::getStringValue(data, IDENTITY_SECTION_NAME, "missingValue").empty(),
                "Missing string lookup should return an empty string");

        bool missingValueThrew = false;
        try {
            IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "missingValue");
        } catch (const std::out_of_range&) {
            missingValueThrew = true;
        }
        require(missingValueThrew, "Typed lookup should reject a missing key");

        bool invalidNumberThrew = false;
        try {
            IniHelper::getValue<int>(data, IDENTITY_SECTION_NAME, "invalidNumericValue");
        } catch (const std::invalid_argument&) {
            invalidNumberThrew = true;
        }
        require(invalidNumberThrew, "Typed lookup should reject trailing non-numeric characters");

        idms::Config config;
        bool missingSectionThrew = false;
        try {
            config.loadFromIniData({{"OTHER", {}}}, IDENTITY_SECTION_NAME);
        } catch (const std::runtime_error&) {
            missingSectionThrew = true;
        }
        require(missingSectionThrew, "Configuration loading should reject a missing section");

        std::cout << "INI helper tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "INI helper test failed: " << error.what() << '\n';
        return 1;
    }
}
