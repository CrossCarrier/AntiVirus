#pragma once

#include "../../HELPERS/include/Types.hpp"

using namespace types::json;
using namespace types::filesystem_types;

namespace index_manager {

    auto update_metaindex(PATH &&path) -> void;

    auto filterModified(const JSON& data , PATHS_CONTAINER& files) -> void;

} // namespace index_manager
