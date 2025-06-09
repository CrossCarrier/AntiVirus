#pragma once

#include "../../HELPERS/include/Types.hpp"

namespace filemanager {
    namespace CTF = types::filesystem_types;

    namespace validate {

        auto validate_file(const CTF::PATH &path) -> bool;
        auto validate_directory(const CTF::PATH &dir_path) -> bool;

    }
    namespace file {

        auto lastModificationTime(const std::filesystem::path &_path) -> time_t;
        auto size(const std::filesystem::path &_path) -> uintmax_t;
        auto hash(const std::filesystem::path &_path) -> std::string;
        auto isMod(const std::filesystem::path &_path, time_t prev_mod, ssize_t prev_size, const std::string &_prev_hash) -> bool;

    } // namespace file
} // namespace filemanager
