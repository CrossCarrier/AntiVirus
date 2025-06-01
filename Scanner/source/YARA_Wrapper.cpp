#include "../include/YARA_Wrapper.hpp"

#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/error.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>

#include <iostream>

namespace {
    auto YARA_CALLBACK_FUNCTION(YR_SCAN_CONTEXT *context, int message, void *message_data, void *user_data) -> int {
    if (message == CALLBACK_MSG_RULE_MATCHING) {
        using RESULTS_DATA = std::vector<std::string>;

        auto *rule = static_cast<YR_RULE *>(message_data);
        auto results = static_cast<RESULTS_DATA *>(user_data);

        results->push_back(std::string(rule->identifier));
    }
    return CALLBACK_CONTINUE;
}
}

namespace yara_wrapper {
    auto YARA_SCAN(const PATH &file , const PATH& rules_config_file, const void* results) -> void {
        yr_initialize();

        YR_RULES *rules = nullptr;
        YR_COMPILER *compiler = nullptr;

        int result = yr_compiler_create(&compiler);
        if (result != ERROR_SUCCESS) {
            std::cout << "Failed to create YARA compiler: " << result << std::endl;
            yr_finalize();
            return;
        }

        FILE *RulesConfigFile = fopen(rules_config_file.c_str(), "r");
        if (!RulesConfigFile) {
            std::cout << "Cannot open rule config file: " << rules_config_file << std::endl;
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        result = yr_compiler_add_file(compiler, RulesConfigFile, nullptr, rules_config_file.filename().c_str());
        fclose(RulesConfigFile);

        if (result != ERROR_SUCCESS) {
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        result = yr_compiler_get_rules(compiler, &rules);
        if (result != ERROR_SUCCESS || rules == nullptr) {
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        result = yr_rules_scan_file(rules, file.c_str(), SCAN_FLAGS_FAST_MODE, YARA_CALLBACK_FUNCTION, const_cast<void *>(results), 0);
        if (result != ERROR_SUCCESS) {
            std::cout << "Failed to scan file: " << file.c_str() << std::endl;
        }

        yr_rules_destroy(rules);
        yr_compiler_destroy(compiler);
        yr_finalize();
    }
}

