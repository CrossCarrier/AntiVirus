#pragma once

#include <filesystem>

namespace config_manager {
    auto init(const std::filesystem::path &__config) -> void;
}