#pragma once

#include "../../HELPERS/include/Types.hpp"

using namespace types::filesystem_types;

namespace config_manager {

    CONFIG_FILES fetch_config_files(const std::filesystem::path &config_file);

} // namespace config_manager
