#pragma once

#include <filesystem>
#include <vector>

namespace config_manager {
    using CONFIG_FILES = std::vector<std::filesystem::path>;

    CONFIG_FILES fetch_config_files(const std::filesystem::path &config_file);
} // namespace config_manager
