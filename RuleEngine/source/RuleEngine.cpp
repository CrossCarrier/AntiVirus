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
#include <thread>
#include <unordered_map>
#include <utility>

#include "../../HELPERS/include/Triplet.hpp"

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

      int YARA_CALLBACK(YR_SCAN_CONTEXT *context, int message, void *message_data,
                        void *user_data) {
            if (message == CALLBACK_MSG_RULE_MATCHING) {
                  auto *rule   = static_cast<YR_RULE *>(message_data);
                  auto results = static_cast<std::vector<std::string> *>(user_data);

                  results->push_back(std::string(rule->identifier));
            }
            return CALLBACK_CONTINUE;
      }

      void YARA_SCAN(const boost::filesystem::path &_file,
                     const boost::filesystem::path &_rules_config_file, const void *_results) {
            yr_initialize();

            YR_RULES *rules;
            YR_COMPILER *compiler;

            yr_compiler_create(&compiler);
            FILE *RulesConfigFile = fopen(_rules_config_file.c_str(), "r");

            yr_compiler_add_file(compiler, RulesConfigFile, nullptr, _file.c_str());
            yr_compiler_get_rules(compiler, &rules);

            fclose(RulesConfigFile);
            auto scan_results = yr_rules_scan_file(rules, _file.c_str(), SCAN_FLAGS_FAST_MODE,
                                                   YARA_CALLBACK, const_cast<void *>(_results), 0);

            yr_rules_destroy(rules);
            yr_compiler_destroy(compiler);
            yr_finalize();
      };

      template <typename STRUCT_TYPE>
      /*HELPER FUNCTION FOR SCANING FILE*/
      void HELPER_ScanOverRulesDir(const boost::filesystem::path &_file,
                                   STRUCT_TYPE &HARDCODED_RULES_DIRS, SCAN_RESULTS *_results) {
            for (const auto &CONFIG : HARDCODED_RULES_DIRS) {
                  boost::filesystem::directory_iterator dir_iter{CONFIG};
                  while (dir_iter != boost::filesystem::directory_iterator{}) {
                        YARA_SCAN(_file, *dir_iter, &_results);
                  }
            }
      }

      /* Checking ANTI_DEBUG_VM, CAPABILITIES, CVE_RULES */
      /* First thread */
      void ScanFirstThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
            /* WARN : THIS FUNCTION CAN BE MADE BETTER! */
            /* UPDATE : FUNCTION CORRECTED */
            using T_RULES_GROUP = Triplet<std::string>;

            T_RULES_GROUP HARDCODED_RULES_DIRS = {RULES_DATABASE.at(RULES_DIRS::ANTI_DEBUG_VM),
                                                  RULES_DATABASE.at(RULES_DIRS::CAPABILITIES),
                                                  RULES_DATABASE.at(RULES_DIRS::CVE_RULES)};

            HELPER_ScanOverRulesDir<T_RULES_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
      }

      /* Checking CRYPTO, EXPOLIT_KITS, MALICIOUS_DOCUMENTS */
      /* Second thread */
      void ScanSecondThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
            using T_RULES_GROUP = Triplet<std::string>;

            T_RULES_GROUP HARDCODED_RULES_DIRS = {
                RULES_DATABASE.at(RULES_DIRS::CRYPTO), RULES_DATABASE.at(RULES_DIRS::EXPLOIT_KITS),
                RULES_DATABASE.at(RULES_DIRS::MALICIOUS_DOCUMENTS)};

            HELPER_ScanOverRulesDir<T_RULES_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
      }

      /* Checking MALWARE, PACKERS, WEB_SHELLS */
      /* Third thread */
      void ScanThirdThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
            using T_RULES_GROUP = Triplet<std::string>;

            T_RULES_GROUP HARDCODED_RULES_DIRS = {RULES_DATABASE.at(RULES_DIRS::MALWARE),
                                                  RULES_DATABASE.at(RULES_DIRS::PACKERS),
                                                  RULES_DATABASE.at(RULES_DIRS::WEB_SHELLS)};

            HELPER_ScanOverRulesDir<T_RULES_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
      }

      /*Checking EMAIL, MALWARE_MOBILE*/
      /*Fourth thread*/
      void ScanFourthThread(const boost::filesystem::path &_file, SCAN_RESULTS *_results) {
            using P_RULES_GROUP = std::pair<std::string, std::string>;

            P_RULES_GROUP HARDCODED_RULES_DIRS = {
                RULES_DATABASE.at(RULES_DIRS::EMAIL),
                RULES_DATABASE.at(RULES_DIRS::MALWARE_MOBILE),
            };

            HELPER_ScanOverRulesDir<P_RULES_GROUP>(_file, HARDCODED_RULES_DIRS, _results);
      }

} // namespace

auto RuleEngine::scan_FILE(const boost::filesystem::path &_file_path) -> SCAN_RESULTS {
      SCAN_RESULTS l_Results;

      std::thread worker_1(ScanFirstThread, _file_path, l_Results);
      std::thread worker_2(ScanSecondThread, _file_path, l_Results);
      std::thread worker_3(ScanThirdThread, _file_path, l_Results);
      std::thread worker_4(ScanFourthThread, _file_path, l_Results);

      return l_Results;
}
