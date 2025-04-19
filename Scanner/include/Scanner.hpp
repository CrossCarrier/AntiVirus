#pragma once

#include <ctime>
#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"

class Scanner {
private:
    time_t m_TimeInterval = 0;

public:
    auto change_time_interval(time_t time_interval) -> void;

    static auto scan_file(const File &_file) -> void;
    static auto scan_directory(const Directory &_directory) -> void;
    static auto full_scan() -> void;
    static auto quick_scan() -> void;
};
