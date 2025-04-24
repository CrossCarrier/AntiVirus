#pragma once

#include <ctime>
#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/Rule.hpp"

class Scanner {
private:
    static auto check_for_string_match() -> ThreatLevel;
    static auto check_for_regex_pattern() -> ThreatLevel;
    static auto check_for_hash_pattern() -> ThreatLevel;
    static auto check_for_behaviour_pattern() -> ThreatLevel;

public:
    static auto scan_file(const File &_file) -> void;
    static auto scan_directory(const Directory &_directory) -> void;
    static auto full_scan() -> void;
    static auto quick_scan() -> void;
};
