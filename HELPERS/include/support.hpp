#pragma once

#include "Types.hpp"

namespace support {
    using namespace types::filesystem_types;
    using namespace types::json;

    namespace filesystem_utils {

        auto load_from_directory(const PATH &path) -> PATHS_CONTAINER;
        auto load_files_from_system() -> PATHS_CONTAINER;

    } // namespace filesystem_utils
    namespace json_utils {

        auto read_data(const PATH &file) -> JSON;
        auto write_data(const PATH &file, const JSON &json) -> void;

    } // namespace json_utils

} // namespace support
