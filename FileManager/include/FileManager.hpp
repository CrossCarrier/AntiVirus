#pragma once

#include "../../HELPERS/include/Types.hpp"

namespace filemanager {
    namespace CTF = types::filesystem_types;

    namespace validate {

        auto validate_file(const CTF::PATH &path) -> bool;

        auto validate_directory(const CTF::PATH &dir_path) -> bool;

    }
    namespace file {

        time_t lastModificationTime(const std::filesystem::path &_path);

        ssize_t size(const std::filesystem::path &_path);

        std::string hash(const std::filesystem::path &_path);

        bool isMod(const std::filesystem::path &_path, time_t prev_mod, ssize_t prev_size, const std::string &_prev_hash);

    } // namespace file
    namespace directory {

        auto loadFiles(const std::filesystem::path &path) -> CTF::PATHS_CONTAINER;

        auto loadModifiedFiles(const std::filesystem::path &_path, nlohmann::json _indexData) -> CTF::PATHS_CONTAINER;

    } // namespace directory
} // namespace filemanager
