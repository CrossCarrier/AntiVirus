#pragma once

#include "../../HELPERS/include/Types.hpp"

#include <algorithm>

using namespace types::filesystem_types;
using namespace types::scanning_types;

namespace scanner {

    template <typename Type1, typename Type2>
    auto operator+=(std::unordered_map<Type1, Type2> &lhs, const std::unordered_map<Type1, Type2> &rhs) -> void {
        std::ranges::for_each(rhs,
                              [&lhs](const std::pair<Type1, Type2> &data) -> void { lhs[data.first] = std::move(data.second); });
    }

    auto scan_file(const PATH &file_path) -> SCAN_RESULTS;

    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK;

} // namespace scanner
