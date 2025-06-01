#pragma once

#include "../../HELPERS/include/Types.hpp"

using namespace types::filesystem_types;

namespace filemanager {
    namespace file {

        time_t lastModificationTime(const std::filesystem::path &_path);

        ssize_t size(const std::filesystem::path &_path);

        std::string hash(const std::filesystem::path &_path);

        bool isMod(const std::filesystem::path &_path, const time_t _prev_mod, const ssize_t _prev_size,
                   const std::string &_prev_hash);

    } // namespace file
    namespace directory {

        PATHS_CONTAINER loadFiles(const std::filesystem::path &__path);

        PATHS_CONTAINER loadModifiedFiles(const std::filesystem::path &_path, nlohmann::json _indexData);

    } // namespace directory
} // namespace filemanager
