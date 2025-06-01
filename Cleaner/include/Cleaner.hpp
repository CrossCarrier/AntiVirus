#pragma once

#include "../../FileManager/include/FileManager.hpp"
#include "../../HELPERS/include/support.hpp"

namespace cleaner {
    static auto removeInfected(const std::string& output_json_path = "output.json") -> void;
    static auto garbage_cleaner() -> void;
};
