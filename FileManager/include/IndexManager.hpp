#pragma once

#include "../../HELPERS/include/Types.hpp"

using namespace types::json;
using namespace types::filesystem_types;

namespace index_manager {

    auto createMetaIndex(const std::string& inputPath, const std::string& nameOutputPath) -> void;
    auto updateMetaIndex(const std::string& inputPath) -> void;

    auto filterModified(PATHS_CONTAINER& files) -> PATHS_CONTAINER;

} // namespace index_manager
