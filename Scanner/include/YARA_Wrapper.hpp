#pragma once

#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/error.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>


#include "../../HELPERS/include/Types.hpp"

using namespace types::filesystem_types;

namespace yara_wrapper {

    auto YARA_SCAN(const PATH &file , const PATH& rules_config_file, const void* results) -> void;

}
