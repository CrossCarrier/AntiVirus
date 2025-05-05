#pragma once
#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"

class Scanner {
private:
    static RuleEngine *supported_RuleEngine;

public:
    static auto access_to_RuleEngine() -> RuleEngine *;
    static auto scan_file(const File &_file, SCAN_RESULTS *results) -> int;

    static auto scan_directory(const Directory &directory) -> int;

    /* FORMAT JSON*/
    /* Build of the file specified in manuals*/
    static auto scan_from_config_file(const boost::filesystem::path &_config_file_path) -> int;
    /* INFO about scanning the system would be avaiable in specified json file*/
    static auto scan_system() -> void;
};