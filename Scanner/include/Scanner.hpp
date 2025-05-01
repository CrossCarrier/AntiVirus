#pragma once
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../../FileManager/include/File.hpp"

using FINAL_RESULTS = std::unordered_map<File, SCAN_RESULTS>;
class Scanner {
private:
    static RuleEngine *supported_RuleEngine;

public:
    static auto access_to_RuleEngine() -> RuleEngine *;
    static auto scan_file(const boost::filesystem::path &_file_path) -> void;

    static auto scan_directory(const boost::filesystem::path &_dir_path) -> void;

    /* FORMAT JSON*/
    /* Build of the file specified in manuals*/
    static auto scan_from_config_file(const boost::filesystem::path _config_file_path) -> void;
    /* INFO about scanning the system would be avaiable in specified json file*/
    static auto scan_system() -> void;
};