#pragma once

#include <unordered_map>
#include <utility>
#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/Rule.hpp"

#define PERCENTAGE_MATCH int

using SingleRuleScan_INFO = std::pair<Rule, PERCENTAGE_MATCH>;

class Scanner {

private:
    static auto check_for_string_match(const File &_file) -> SingleRuleScan_INFO;
    static auto check_for_regex_pattern(const File &_file) -> SingleRuleScan_INFO;
    static auto check_for_hash_pattern(const File &_file) -> SingleRuleScan_INFO;
    static auto check_for_behaviour_pattern(const File &_file) -> SingleRuleScan_INFO;

public:
    static auto scan_file(const File &_file) -> void;
    static auto scan_directory(const Directory &_directory) -> void;
    static auto full_scan() -> void;
    static auto quick_scan() -> void;
};
