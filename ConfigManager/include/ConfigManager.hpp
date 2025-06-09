#pragma once

#include "../../HELPERS/include/Types.hpp"

namespace config_manager {
    namespace CTF = types::filesystem_types;

    auto fetch_config_files(const std::filesystem::path& config_file_path) -> CTF::PATHS_CONTAINER;

    auto change_number_of_threads(int newNumberOfThreads) -> void;

    auto set_index_storage_location(const CTF::PATH& new_location) -> void;
    auto set_output_file(const CTF::PATH &new_location) -> void;

    auto get_number_of_threads() -> int;
    auto get_index_storage_path() -> std::string;
    auto get_output_file_path() -> std::string;

} // namespace config_manager
