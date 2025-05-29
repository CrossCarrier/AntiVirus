#pragma once

#include <filesystem>
#include <vector>
#include "../../HELPERS/include/json.hpp"

namespace filemanager {
    namespace file {
        time_t lastModificationTime(const std::filesystem::path &_path);
        ssize_t size(const std::filesystem::path &_path);
        std::string hash(const std::filesystem::path &_path);

        bool isMod(const std::filesystem::path &_path, const time_t _prev_mod, const ssize_t _prev_size,
                   const std::string &_prev_hash);
    } // namespace file
    namespace directory {
        using PATHS_CONTAINER = std::vector<std::filesystem::path>;
        using DIRECTORY_ITER_R = std::filesystem::recursive_directory_iterator;
        using ITER_OPTIONS = std::filesystem::directory_options;

        PATHS_CONTAINER loadFiles(const std::filesystem::path &__path);
        PATHS_CONTAINER loadModifiedFiles(const std::filesystem::path &_path, nlohmann::json _indexData);
    } // namespace directory
} // namespace filemanager
