#include "../include/YARA_Wrapper.hpp"
#include "../../Logger/include/Logger.hpp"
#include <format>

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
            auto& logger = logger::Logger::getInstance();
            using RESULTS_DATA = std::vector<std::string>;

            auto *rule = static_cast<YR_RULE *>(message_data);
            auto results = static_cast<RESULTS_DATA *>(user_data);

            logger.debug(std::format("YARA match found: rule '{}'", rule->identifier));
            results->push_back(std::string(rule->identifier));
        }
        return CALLBACK_CONTINUE;
    }
}

namespace yara_wrapper {
    auto YARA_SCAN(const PATH &file, const PATH& rules_config_file, const void* results) -> void {
        auto& logger = logger::Logger::getInstance();
        logger.debug(std::format("Starting YARA scan for file: {} using rules: {}",
                               file.string(), rules_config_file.string()));

        logger.debug("Initializing YARA library");
        int result = yr_initialize();
        if (result != ERROR_SUCCESS) {
            logger.error(std::format("Failed to initialize YARA library: error code {}", result));
            return;
        }

        YR_RULES *rules = nullptr;
        YR_COMPILER *compiler = nullptr;

        logger.debug("Creating YARA compiler");
        result = yr_compiler_create(&compiler);
        if (result != ERROR_SUCCESS) {
            logger.error(std::format("Failed to create YARA compiler: error code {}", result));
            yr_finalize();
            return;
        }

        logger.debug(std::format("Opening rule file: {}", rules_config_file.string()));
        FILE *RulesConfigFile = fopen(rules_config_file.c_str(), "r");
        if (!RulesConfigFile) {
            logger.error(std::format("Cannot open rule file: {}", rules_config_file.string()));
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        logger.debug(std::format("Adding rule file to compiler: {}", rules_config_file.filename().string()));
        result = yr_compiler_add_file(compiler, RulesConfigFile, nullptr, rules_config_file.filename().c_str());
        fclose(RulesConfigFile);
        logger.debug("Rule file closed after reading");

        if (result != ERROR_SUCCESS) {
            logger.error(std::format("Failed to compile rules from {}: error code {}",
                                  rules_config_file.string(), result));
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        logger.debug("Getting compiled rules from compiler");
        result = yr_compiler_get_rules(compiler, &rules);
        if (result != ERROR_SUCCESS || rules == nullptr) {
            logger.error(std::format("Failed to get compiled rules: error code {}", result));
            yr_compiler_destroy(compiler);
            yr_finalize();
            return;
        }

        logger.info(std::format("Scanning file with YARA: {}", file.string()));
        result = yr_rules_scan_file(rules, file.c_str(), SCAN_FLAGS_FAST_MODE,
                                  YARA_CALLBACK_FUNCTION, const_cast<void *>(results), 0);

        if (result != ERROR_SUCCESS) {
            logger.error(std::format("YARA scan failed for {}: error code {}", file.string(), result));
        } else {
            logger.debug(std::format("YARA scan completed successfully for: {}", file.string()));
        }

        logger.debug("Cleaning up YARA resources");
        yr_rules_destroy(rules);
        yr_compiler_destroy(compiler);
        yr_finalize();

        logger.debug(std::format("YARA scan operation finished for: {}", file.string()));
    }
}