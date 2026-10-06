#pragma once

#include <map>
#include <string>

/**
 * @file def_helper.hpp
 * Helper class for defining common types used in the project.
 */
namespace helper_tool {
    class DefHelper {
    public:
        typedef std::map<std::string, std::map<std::string, std::string>> ConfigMap;;
    };
}