#pragma once

#include "../../HELPERS/include/Types.hpp"

namespace rule_engine {
    using namespace types::rules_types;
    using namespace types::json;
    using namespace types::filesystem_types;

    auto get_Rules() -> RULES_PACK;

}

