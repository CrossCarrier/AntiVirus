#pragma once

#include "../../HELPERS/include/Types.hpp"

using namespace types::rules_types;
using namespace types::json;
using namespace types::filesystem_types;

namespace rule_engine {

    auto get_Rules() -> RULES_PACK;

}

