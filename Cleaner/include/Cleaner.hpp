#pragma once

#include "../../FileManager/include/File.hpp"
#include "../../DirectoryManager/include/Directory.hpp"

class Cleaner{
public:
    static auto clean_file(const File& _file) -> void; /* system("rm (path file)")*/
    static auto clean_directory(const Directory& _directory) -> void; /*itterate over dir and check for unsafe files and clean them up*/
    static auto full_clean() -> void;
    static auto garbage_cleaner() -> void;
};