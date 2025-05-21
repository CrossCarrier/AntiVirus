#pragma once
#include "../../FileManager/include/File.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace scanner {
    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS;
    auto scan_directory(const std::filesystem::path &directory_path) -> std::vector<SCAN_RESULTS>;

    auto quick_scan() -> void;
    auto full_scan() -> void;

    auto scan_system() -> void;
} // namespace scanner