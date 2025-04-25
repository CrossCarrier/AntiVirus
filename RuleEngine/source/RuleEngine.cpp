#include "../include/RuleEngine.hpp"
#include <yara.h>
#include <iostream>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

int yara_callback(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data) {
    if (message == CALLBACK_MSG_RULE_MATCHING) {
        auto *rule = static_cast<YR_RULE *>(message_data);

        std::cout << "MATCHED! -> Rule : " << rule->identifier << std::endl;

        auto *results = static_cast<ScanResults *>(user_data);
        results->push_back(MatchResult{rule->identifier});
    }
    return CALLBACK_CONTINUE;
}

auto RuleEngine::runYaraScan(const boost::filesystem::path &_file, const boost::filesystem::path &_rules_config_file, const void* _results)
        -> void {
    yr_initialize();

    YR_RULES *rules;
    YR_COMPILER *compiler;

    yr_compiler_create(&compiler);
    FILE *ruleFile = fopen(_rules_config_file.c_str(), "r");

    yr_compiler_add_file(compiler, ruleFile, nullptr, _file.c_str());
    yr_compiler_get_rules(compiler, &rules);

    fclose(ruleFile);

    yr_rules_scan_file(rules, _file.c_str(), 0, yara_callback, &_results , 0);

    yr_rules_destroy(rules);
    yr_compiler_destroy(compiler);
    yr_finalize();
}
