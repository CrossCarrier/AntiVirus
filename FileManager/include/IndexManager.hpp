#pragma once
#include "../../FileManager/include/File.hpp"
#include <filesystem>
#include <vector>

namespace index_manager {
    namespace FILE = std::filesystem;
    using FILE_PACK = std::vector<File>;
    using PATHS_CONTAINER = std::vector<std::filesystem::path>;

    auto fetch_data(FILE::path &&__path, const bool __mod) -> FILE_PACK;
    auto fetch_data(FILE::path &&__path, std::vector<std::filesystem::path> &&files) -> FILE_PACK;
    auto update_metaindex(FILE::path &&_path) -> void;

    auto filter_modified(std::vector<std::filesystem::path> &&files) -> PATHS_CONTAINER;
} // namespace index_manager
