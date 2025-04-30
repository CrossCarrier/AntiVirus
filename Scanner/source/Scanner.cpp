#include "../include/Scanner.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "../../HELPERS/include/FixedSizeContainer.hpp"
#include <boost/filesystem/operations.hpp>
namespace {
    using DOUBLE_GROUP = FixedSizeContainer<std::string>;
    using TRIPLE_GROUP = FixedSizeContainer<std::string>;

    template <typename STRUCT_TYPE>
    /*HELPER FUNCTION FOR SCANING FILE*/
    void HELPER_ScanOverRulesDir(const boost::filesystem::path &_file,
                                 STRUCT_TYPE &HARDCODED_RULES_DIRS, SCAN_RESULTS *_results) {
          for (const auto &CONFIG : HARDCODED_RULES_DIRS) {
                boost::filesystem::directory_iterator dir_iter{CONFIG};
                while (dir_iter != boost::filesystem::directory_iterator{}) {
                      YARA_Wrapper::YARA_SCAN(_file, *dir_iter, &_results);
                }
          }
    }

    /* Checking ANTI_DEBUG_VM, CAPABILITIES, CVE_RULES */
    /* First thread */
    void ScanFirstThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
          /* WARN : THIS FUNCTION CAN BE MADE BETTER! */
          /* UPDATE : FUNCTION CORRECTED */
          TRIPLE_GROUP HARDCODED_RULES_DIRS(3);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::ANTI_DEBUG_VM)
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::CAPABILITIES);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::CVE_RULES);

          HELPER_ScanOverRulesDir<TRIPLE_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
    }

    /* Checking CRYPTO, EXPOLIT_KITS, MALICIOUS_DOCUMENTS */
    /* Second thread */
    void ScanSecondThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
          TRIPLE_GROUP HARDCODED_RULES_DIRS(3);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::CRYPTO)
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::EXPLOIT_KITS);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::MALICIOUS_DOCUMENTS);

          HELPER_ScanOverRulesDir<TRIPLE_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
    }

    /* Checking MALWARE, PACKERS, WEB_SHELLS */
    /* Third thread */
    void ScanThirdThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
          TRIPLE_GROUP HARDCODED_RULES_DIRS(3);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::MALWARE)
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::PACKERS);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::WEB_SHELLS);

          HELPER_ScanOverRulesDir<TRIPLE_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
    }

    /*Checking EMAIL, MALWARE_MOBILE*/
    /*Fourth thread*/
    void ScanFourthThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
          DOUBLE_GROUP HARDCODED_RULES_DIRS(2);
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::EMAIL)
          HARDCODED_RULES_DIRS << Scanner::access_to_RuleEngine()->get_RULES(RULES_DIRS::MALWARE_MOBILE);

          HELPER_ScanOverRulesDir<DOUBLE_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
    }
}

auto Scanner::access_to_RuleEngine() -> RuleEngine* { return supported_RuleEngine;}
