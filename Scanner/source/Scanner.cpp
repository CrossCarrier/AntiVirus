#include "../include/Scanner.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include <exception>
#include <future>
#include <iostream>

#include "FileManager.hpp"

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

        std::unique_ptr<ThreadPool> internal_rule_pool = std::make_unique<ThreadPool>(threads_for_rules_processing);

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

    auto scanMultipleFilesHELPER_ThreadSafe(const PATHS_CONTAINER& filesPack, SCAN_RESULTS_PACK& sharedResults, std::mutex &resultsMutex, int threads_for_rules_processing_per_file,
        const PATHS_CONTAINER& rules) -> void {

        std::unique_ptr<ThreadPool> innerThreadPool = std::make_unique<ThreadPool>(threads_for_rules_processing_per_file);

        std::ranges::for_each(filesPack, [&](const PATH &filePath) -> void {
            SCAN_RESULTS fileScanningResults;
            try {
                fileScanningResults = scanSingleFile(filePath, threads_for_rules_processing_per_file, rules);
            } catch (const std::exception& e) {
                std::cerr << "Unhandled exception while scanning file " << filePath.string() << ": " << e.what() << std::endl;
                return;
            }

            if (!fileScanningResults.empty()) {
                std::lock_guard<std::mutex> lock(resultsMutex);
                sharedResults[filePath.string()] = std::move(fileScanningResults);
            }
        });
    }
} // namespace

namespace scanner {
    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK {
        auto fetchedRules = fetch_all_rules();

        const auto threadsToUse = std::max(1, numberOfThreads);
        ThreadPool pool(threadsToUse);
        SCAN_RESULTS_PACK results;
        std::mutex resultsMutex;

        auto vectors = support::container_utils::split(files, threadsToUse);

        std::vector<std::future<void>> taskFutures;
        taskFutures.reserve(vectors.size());

        std::ranges::for_each(vectors, [&](const auto &filesPack) -> void {
            if (!filesPack.empty()) {
                taskFutures.emplace_back(pool.addTask(scanMultipleFilesHELPER_ThreadSafe,
                    filesPack,
                    std::ref(results),
                    std::ref(resultsMutex)
                    ));
            }
        });

        std::ranges::for_each(taskFutures, [](auto& currentFuture) -> void {
            try {
                currentFuture.get();
            } catch (std::exception& _) {
                // Logging error logic
                throw;
            }
        });

        return results;
    }
} // namespace scanner
