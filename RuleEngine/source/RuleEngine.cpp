#include "../include/RuleEngine.hpp"
#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>

#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstdarg>
#include <string>
#include <unordered_map>

namespace {

      using RULES_DIRECTORIES = std::unordered_map<RULES_DIRS, std::string>;
      /*
                  ASSIGNMENT OF RULES DIRECTORIES
      */
      RULES_DIRECTORIES RULES_DATABASE = {
          {RULES_DIRS::ANTI_DEBUG_VM, "../../rules-master/antidebug_antivm"},
          {RULES_DIRS::CAPABILITIES, "../../rules-master/capabilities"},
          {RULES_DIRS::CVE_RULES, "../../rules-master/cve_rules"},
          {RULES_DIRS::CRYPTO, "../../antivirus/rules-master/crypto"},
          {RULES_DIRS::EXPLOIT_KITS, "../../antivirus/rules-master/exploit_kits"},
          {RULES_DIRS::MALICIOUS_DOCUMENTS, "../../antivirus/rules-master/maldocs"},
          {RULES_DIRS::MALWARE, "../../antivirus/rules-master/malware"},
          {RULES_DIRS::PACKERS, "../../antivirus/rules-master/packers"},
          {RULES_DIRS::WEB_SHELLS, "../../antivirus/rules-master/webshells"},
          {RULES_DIRS::EMAIL, "../../antivirus/rules-master/email"},
          {RULES_DIRS::MALWARE_MOBILE, "../../antivirus/rules-master/mobile_malware"}};

} // namespace
