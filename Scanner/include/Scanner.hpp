#pragma once

#include "../../FileManager/include/File.hpp"
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace scanner {
    template <typename Type1, typename Type2>
    auto operator+=(std::unordered_map<Type1, Type2> &lhs, const std::unordered_map<Type1, Type2> &rhs) -> void {
        std::ranges::for_each(rhs,
                              [&lhs](const std::pair<Type1, Type2> &data) -> void { lhs[data.first] = std::move(data.second); });
    }

    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS;
    auto scanMultipleFiles(const std::vector<std::filesystem::path> &files, int numberOfThreads)
        -> std::unordered_map<std::string, SCAN_RESULTS>;
} // namespace scanner
