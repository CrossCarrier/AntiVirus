#pragma once

#include <filesystem>
#include <vector>

#include "../../FileManager/include/File.hpp"

class Directory {
public:
    Directory(boost::filesystem::path working_directory_path);

    auto get_Files() const noexcept -> const std::vector<File> &;

private:
    std::vector<File> m_Files;
};
