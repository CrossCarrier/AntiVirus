#pragma once

#include <boost/filesystem/path.hpp>
#include <filesystem>
#include <utility>
#include <vector>
#include "../../FileManager/include/File.hpp"

#define ANTI_DEBUG_VM "../rules-master/antidebug_antivm_index.yar"
#define CAPABILITIES "../rules-master/capabilities_index.yar"
#define CVE_RULES "../rules-master/cve_rules_index.yar"
#define CRYPTO "../rules-master/crypto_index.yar"
#define EXPLOIT_KITS "../rules-master/exploit_kits_index.yar"
#define MALICIOUS_DOCUMENTS "../rules-master/maldocs_index.yar"
#define MALWARE "../rules-master/malware_index.yar"
#define PACKERS "../rules-master/packers_index.yar"
#define WEB_SHELLS "../rules-master/webshells_index.yar"
#define EMAIL "../rules-master/email_index.yar"
#define MALWARE_MOBILE "../rules-master/mobile_malware_index.yar"

struct MatchResult {
    std::string rule_name;
};

using ScanResults = std::vector<MatchResult>;

using RULES_DATABASE = std::vector<Rule>;

class RuleEngine {
private:
    static boost::filesystem::path m_ConfigFile;

public:
    static auto runYaraScan(const boost::filesystem::path &_file, const boost::filesystem::path &_rules_config_file,
                            const void* _results) -> void;
};
