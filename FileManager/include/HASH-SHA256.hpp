#pragma once

#include <filesystem>

namespace hash_SHA256 {
    auto hash_file(const std::filesystem::path &file_path) -> std::string;
};
