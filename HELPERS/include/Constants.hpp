#pragma once

#include <unordered_set>
#include <string>

namespace constants {

    extern const std::unordered_set<std::string> CriticalSystemLocalisation;
    extern const std::unordered_set<std::string> UserDirectories;
    extern const std::unordered_set<std::string> TemporaryData;
    extern const std::unordered_set<std::string> AutoStartLocations;
    extern const std::unordered_set<std::string> WebDirectories;
    extern const std::unordered_set<std::string> ExcludeDirectories;

}