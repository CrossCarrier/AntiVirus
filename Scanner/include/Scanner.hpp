#pragma once

#include "../../HELPERS/include/Types.hpp"

namespace scanner {

    using namespace types::scanning_types;
    using namespace types::filesystem_types;

    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK;

} // namespace scanner
