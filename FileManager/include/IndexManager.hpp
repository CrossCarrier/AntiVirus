#pragma once
#include "../../FileManager/include/File.hpp"
#include <filesystem>
#include <vector>

namespace index_manager {
    namespace FILE = std::filesystem;
    using FILE_PACK = std::vector<File>;

    auto fetch_data(FILE::path &&__path, const bool __mod) -> FILE_PACK;
    auto update_metaindex(FILE::path &&_path) -> void;
} // namespace index_manager