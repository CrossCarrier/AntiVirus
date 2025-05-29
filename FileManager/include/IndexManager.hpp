#pragma once
#include <filesystem>
#include <vector>
#include "../../HELPERS/include/json.hpp"

namespace index_manager {
    namespace FILE = std::filesystem;
    using PATHS_CONTAINER = std::vector<std::filesystem::path>;

    auto update_metaindex(FILE::path &&path) -> void;
    auto filterModified(const nlohmann::json& data , PATHS_CONTAINER& files) -> void;
} // namespace index_manager
