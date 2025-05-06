#pragma once

#include <vector>

#include "../../FileManager/include/File.hpp"
#include "../../HELPERS/include/quattro.hpp"
class Directory {
public:
    Directory(boost::filesystem::path working_directory_path);

    auto get_Files() const noexcept -> const std::vector<File> &;

private:
    QuattroList<File> files;
};
