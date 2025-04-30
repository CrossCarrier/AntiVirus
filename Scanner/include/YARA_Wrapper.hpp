#pragma once

#include <boost/filesystem/filesystem.hpp>

#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>

#include <string>
#include <vector>

class YARA_Wrapper{
    private:
    static auto YARA_CALLBACK_FUNCTION(YR_SCAN_CONTEXT *context, int message, void* message_data, void* user_data) -> int {
        if (message == CALLBACK_MSG_RULE_MATCHING) {
            using RESULTS_DATA = std::vector<std::string>;

            auto *rule   = static_cast<YR_RULE *>(message_data);
            auto results = static_cast<std::vector<RESULTS_DATA *>(user_data);

            results->push_back(std::string(rule->identifier));
            }
            return CALLBACK_CONTINUE;
    }

    public:
    static auto YARA_SCAN(const boost::filesystem::path& _file, const boost::filesystem::path &_rules_config_file, const void *_results) {
    {
            yr_initialize();
    
            YR_RULES *rules;
            YR_COMPILER *compiler;
    
            yr_compiler_create(&compiler);
            FILE *RulesConfigFile = fopen(_rules_config_file.c_str(), "r");
    
            yr_compiler_add_file(compiler, RulesConfigFile, nullptr, _file.c_str());
            yr_compiler_get_rules(compiler, &rules);
    
            fclose(RulesConfigFile);
            auto scan_results = yr_rules_scan_file(rules, _file.c_str(), SCAN_FLAGS_FAST_MODE,
                                            YARA_CALLBACK_FUNCTION, const_cast<void *>(_results), 0);
    
            yr_rules_destroy(rules);
            yr_compiler_destroy(compiler);
            yr_finalize();
    }
};