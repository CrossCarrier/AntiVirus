#include "../include/Scanner.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include <exception>
#include <future>
#include <iostream>

#include "CLI/Validators.hpp"

namespace {
    auto fetch_all_rules() -> PATHS_CONTAINER {
        PATHS_CONTAINER loaded_rules;

        for (const auto &rule_directory_path : rule_engine::get_Rules()) {
            if (!std::filesystem::exists(rule_directory_path) || !std::filesystem::is_directory(rule_directory_path)) {
                std::cerr << "Rule directory path " << rule_directory_path << " does not exist or is not a directory!" << std::endl;
                continue;
            }

            try {
                for (const auto &rule_file_entry : DIRECTORY_ITER(rule_directory_path)) {
                    if (rule_file_entry.is_regular_file() && rule_file_entry.path().extension() == ".yar") {
                        loaded_rules.push_back(rule_file_entry.path());
                    }
                }
            } catch (const std::filesystem::filesystem_error& fs_err) {
                std::cerr << "Filesystem error iterating rule directory " << rule_directory_path << ": " << fs_err.what() << std::endl;
            }
        }

        return loaded_rules;
    }

    auto scanSingleFile(const PATH &file_path, int threads_for_rules_processing, const PATHS_CONTAINER& rules) -> SCAN_RESULTS {
        SCAN_RESULTS aggregated_file_results;
        std::mutex aggregated_results_mutex;

        auto yara_scan_task_lambda = [&](const PATH& individual_rule_file) -> void {
            SCAN_RESULTS task_scan_results;
            try {
                std::lock_guard<std::mutex> lock(aggregated_results_mutex);
                yara_wrapper::YARA_SCAN(file_path, individual_rule_file, &aggregated_file_results);

            } catch (const std::exception &e) {
                std::cerr << "Error scanning file " << file_path.string() << " with rule " << individual_rule_file.string() << ": " << e.what() << std::endl;
            }
        };

        std::ranges::for_each(rules, [&](const std::filesystem::path& individualRuleFile) -> void {
            yara_scan_task_lambda(individualRuleFile);
        });

        return aggregated_file_results;
    }

}

namespace scanner {
    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK {
        const auto fetchedRules = fetch_all_rules();

        const auto threadsToUse = std::max(1, numberOfThreads);
        const auto pool = std::make_unique<ThreadPool>(threadsToUse);
        SCAN_RESULTS_PACK results;
        std::mutex resultsMutex;

        std::unordered_map<std::string, std::future<SCAN_RESULTS>> taskFutures;

        std::ranges::for_each(files, [&](const PATH &filePath) -> void {
            taskFutures.insert({std::move(filePath.string()), pool->addTask(scanSingleFile, filePath, threadsToUse, fetchedRules)});
        });

        for (auto& [fst, snd] : taskFutures) {
            try {
                auto scanResults = std::move(snd.get());

                if (!scanResults.empty()) {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    results.insert({fst, std::move(scanResults)});
                }

            } catch (const std::future_error& fe) {
                std::cerr << "Future error collecting scan result for file '" << fst << "': " << fe.what() << " (code: " << fe.code() << ")" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Exception collecting scan result for file '" << fst << "': " << e.what() << std::endl;
            }
        }


        return results;
    }
} // namespace scanner
