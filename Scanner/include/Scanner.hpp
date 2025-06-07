#pragma once

#include "../../HELPERS/include/Types.hpp"

#include <algorithm>

using namespace types::filesystem_types;
using namespace types::scanning_types;

namespace scanner {

    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK;

} // namespace scanner
