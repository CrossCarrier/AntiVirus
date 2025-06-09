#pragma once

#include "../../HELPERS/include/Types.hpp"


namespace yara_wrapper {

    using namespace types::filesystem_types;

    auto YARA_SCAN(const PATH &file , const PATH& rules_config_file, const void* results) -> void;

}
