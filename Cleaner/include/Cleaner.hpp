#pragma once

#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"

class Cleaner {
public:
    static auto clean_file(const File &_file) -> void;
    static auto clean_directory(const Directory &_directory) -> void;
    static auto full_clean() -> void;
    static auto quick_clean() -> void;
    static auto garbage_cleaner() -> void;
};
