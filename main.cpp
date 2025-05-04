#include "FileManager/include/File.hpp"
#include "HELPERS/include/json_manager.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

int main() {
    boost::filesystem::path file_path("example.json");
    boost::filesystem::path index_file("index_file.json");
    nlohmann::json json_data;
    File test_file(file_path);

    std::cout << test_file.get_FilePath() << std::endl;

    return 0;
}
