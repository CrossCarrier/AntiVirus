#pragma once

#include <boost/filesystem/path.hpp>
#include "../include/json.hpp"
#include "../../FileManager/include/File.hpp"

namespace json_manager {
    using json = nlohmann::json;

    auto read_data(const boost::filesystem::path &_json_file_path) -> json;
    auto write_data(const boost::filesystem::path &_json_file_path, const File& _file) -> void;
}