#include "../include/json_manager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace json_manager {
    auto read_data(const boost::filesystem::path &_json_file_path) -> json {
        if (!boost::filesystem::exists(_json_file_path)) {
            std::stringstream error_message;
            error_message << "Path not found: " << _json_file_path.c_str();
            throw PathNotFound(error_message.str());
        }

        std::ifstream input_file(_json_file_path.c_str());
        if (!input_file) {
            throw std::invalid_argument("Error occurred while opening a file!\n");
        }

        json json_data;
        try {
            json_data = json::parse(input_file);
        } catch (std::exception &error) {
            std::cout << error.what() << std::endl;
        }

        return json_data;
    }
    auto write_data(const boost::filesystem::path &_json_file_path, const File &_file) -> void {
        if (!boost::filesystem::exists(_json_file_path)) {
            std::stringstream error_message;
            error_message << "Path not found: " << _json_file_path.c_str();
            throw PathNotFound(error_message.str());
        }

        json json_data;
        auto l_file_path = static_cast<std::string>(_file.get_FilePath().c_str());

        json_data[l_file_path]["Modification Time"] = _file.get_LastModificationTime();
        json_data[l_file_path]["SHA256 Hash"]       = _file.get_HashID();
        json_data[l_file_path]["Size : "]           = _file.get_Size();

        std::ofstream output_file(_json_file_path.c_str());
        if (!output_file) {
            throw std::invalid_argument("Error occurred while opening a file!\n");
        }

        output_file << std::setw(4) << json_data;
    }

} // namespace json_manager