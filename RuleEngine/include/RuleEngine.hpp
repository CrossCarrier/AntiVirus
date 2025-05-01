#pragma once

#include <boost/filesystem/path.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using SCAN_RESULTS = std::vector<std::string>;

enum class RULES_DIRS_YARA : uint8_t {
    ANTI_DEBUG_VM       = 1,
    CAPABILITIES        = 2,
    CVE_RULES           = 3,
    CRYPTO              = 4,
    EXPLOIT_KITS        = 5,
    MALICIOUS_DOCUMENTS = 6,
    MALWARE             = 7,
    PACKERS             = 8,
    WEB_SHELLS          = 9,
    EMAIL               = 10,
    MALWARE_MOBILE      = 11
};

using RULES_DIRECTORIES = std::unordered_map<RULES_DIRS_YARA, std::string>;

class RuleEngine {
private:
    static RULES_DIRECTORIES *supported_rules;

public:
    static auto getRULES() noexcept -> RULES_DIRECTORIES &;
    static auto add_RULE(const boost::filesystem::path &_new_rule_config_file,
                         const RULES_DIRS_YARA &_new_rule_type) -> void;
    static auto remove_RULE(const boost::filesystem::path &_removal_rule) -> void;
};
