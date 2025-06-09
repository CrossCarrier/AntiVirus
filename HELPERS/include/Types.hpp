#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>
#include "json.hpp"
#include <unordered_set>

namespace types {

    namespace filesystem_types {

        using PATH = std::filesystem::path;
        using PATHS_CONTAINER = std::vector<PATH>;
        using CONFIG_FILES = std::vector<PATH>;
        using UNIQUE_CONFIG_FILES = std::unordered_set<PATH>;
        using DIRECTORY_ITER_R = std::filesystem::recursive_directory_iterator;
        using DIRECTORY_ITER = std::filesystem::directory_iterator;
        using ITER_OPTIONS = std::filesystem::directory_options;

    }

    namespace scanning_types {

        using SCAN_RESULTS = std::vector<std::string>;
        using SCAN_RESULTS_PACK = std::unordered_map<std::string, SCAN_RESULTS>;
        using FILE_SCAN_DATA = std::pair<std::string, SCAN_RESULTS>;

    }

    namespace json {

        using JSON = nlohmann::json;

    }

    namespace rules_types {

        using RULES_PACK = std::vector<std::string>;

    }
}