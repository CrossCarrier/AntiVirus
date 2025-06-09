#include "../include/Scanner.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include "../../Logger/include/Logger.hpp"
#include <exception>
#include <future>
#include <iostream>
#include <format>

namespace {
    using namespace types::scanning_types;
    using namespace types::filesystem_types;

    auto fetch_all_rules() -> PATHS_CONTAINER {
        auto& logger = logger::Logger::getInstance();
        logger.info("Starting to fetch all YARA rules");

        PATHS_CONTAINER loaded_rules;
        auto rule_directories = rule_engine::get_Rules();
        logger.debug(std::format("Received {} rule directories to process", rule_directories.size()));

        size_t processedDirs = 0;
        size_t validDirs = 0;
        size_t rulesFound = 0;

        for (const auto &rule_directory_path : rule_directories) {
            processedDirs++;
            logger.debug(std::format("Processing rule directory {}/{}: {}",
                processedDirs, rule_directories.size(), rule_directory_path));

            if (!std::filesystem::exists(rule_directory_path) || !std::filesystem::is_directory(rule_directory_path)) {
                logger.warn(std::format("Rule directory path {} does not exist or is not a directory", rule_directory_path));
                continue;
            }

            validDirs++;
            logger.debug(std::format("Valid rule directory found: {}", rule_directory_path));

            try {
                size_t dirRuleCount = 0;
                logger.debug(std::format("Scanning directory for .yar files: {}", rule_directory_path));

                for (const auto &rule_file_entry : DIRECTORY_ITER(rule_directory_path)) {
                    if (rule_file_entry.is_regular_file() && rule_file_entry.path().extension() == ".yar") {
                        logger.debug(std::format("Found YARA rule file: {}", rule_file_entry.path().string()));
                        loaded_rules.push_back(rule_file_entry.path());
                        dirRuleCount++;
                        rulesFound++;
                    }
                }

                logger.debug(std::format("Found {} rule files in directory {}", dirRuleCount, rule_directory_path));
            } catch (const std::filesystem::filesystem_error& fs_err) {
                logger.error(std::format("Filesystem error iterating rule directory {}: {}",
                    rule_directory_path, fs_err.what()));
            }
        }

        logger.success(std::format("Completed rule loading: {} rules found in {}/{} valid directories",
            rulesFound, validDirs, processedDirs));
        return loaded_rules;
    }

    auto scanSingleFile(const PATH &file_path, int threads_for_rules_processing, const PATHS_CONTAINER& rules) -> SCAN_RESULTS {
        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Starting scan of file: {}", file_path.string()));
        logger.debug(std::format("Using {} threads for rules processing with {} rules",
            threads_for_rules_processing, rules.size()));

        SCAN_RESULTS aggregated_file_results;
        std::mutex aggregated_results_mutex;

        size_t processedRules = 0;
        size_t successfulScans = 0;
        size_t failedScans = 0;

        auto yara_scan_task_lambda = [&](const PATH& individual_rule_file) -> void {
            logger.debug(std::format("Applying rule file to {}: {}",
                file_path.string(), individual_rule_file.string()));

            try {
                std::lock_guard<std::mutex> lock(aggregated_results_mutex);

                logger.debug(std::format("Starting YARA scan with rule: {}", individual_rule_file.string()));
                yara_wrapper::YARA_SCAN(file_path, individual_rule_file, &aggregated_file_results);

                successfulScans++;
                logger.debug(std::format("Successfully applied rule {} to {}",
                    individual_rule_file.string(), file_path.string()));

            } catch (const std::exception &e) {
                failedScans++;
                logger.error(std::format("Error scanning file {} with rule {}: {}",
                    file_path.string(), individual_rule_file.string(), e.what()));
            }
        };

        logger.debug(std::format("Starting sequential rule processing for {}", file_path.string()));
        std::ranges::for_each(rules, [&](const std::filesystem::path& individualRuleFile) -> void {
            processedRules++;
            yara_scan_task_lambda(individualRuleFile);
        });

        logger.info(std::format("Scan completed for {}: {} rules processed ({} successful, {} failed)",
            file_path.string(), processedRules, successfulScans, failedScans));

        logger.debug(std::format("Scan found {} matches for file {}",
            aggregated_file_results.size(), file_path.string()));

        return aggregated_file_results;
    }
}

namespace scanner {
    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads) -> SCAN_RESULTS_PACK {
        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Starting multi-file scan of {} files using {} threads",
            files.size(), numberOfThreads));

        logger.debug("Fetching YARA rules for scanning");
        const auto fetchedRules = fetch_all_rules();
        logger.debug(std::format("Fetched {} rules for scanning", fetchedRules.size()));

        const auto threadsToUse = std::max(1, numberOfThreads);
        logger.debug(std::format("Creating thread pool with {} threads", threadsToUse));
        const auto pool = std::make_unique<ThreadPool>(threadsToUse);

        SCAN_RESULTS_PACK results;
        std::mutex resultsMutex;

        std::unordered_map<std::string, std::future<SCAN_RESULTS>> taskFutures;

        logger.info("Submitting scan tasks to thread pool");
        size_t tasksSubmitted = 0;
        std::ranges::for_each(files, [&](const PATH &filePath) -> void {
            logger.debug(std::format("Submitting scan task for file: {}", filePath.string()));
            taskFutures.insert({std::move(filePath.string()),
                pool->addTask(scanSingleFile, filePath, threadsToUse, fetchedRules)});
            tasksSubmitted++;
        });

        logger.info(std::format("Submitted {} scan tasks to thread pool", tasksSubmitted));
        logger.debug("Collecting scan results from futures");

        size_t processedResults = 0;
        size_t successfulResults = 0;
        size_t failedResults = 0;
        size_t positiveScanResults = 0;

        for (auto& [filePath, future] : taskFutures) {
            processedResults++;
            logger.debug(std::format("Collecting result {}/{} for file: {}",
                processedResults, taskFutures.size(), filePath));

            try {
                auto scanResults = std::move(future.get());
                successfulResults++;

                if (!scanResults.empty()) {
                    logger.info(std::format("Positive scan result for file: {} (found {} matches)",
                        filePath, scanResults.size()));
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    results.insert({filePath, std::move(scanResults)});
                    positiveScanResults++;
                } else {
                    logger.debug(std::format("No matches found for file: {}", filePath));
                }

            } catch (const std::future_error& fe) {
                failedResults++;
                logger.error(std::format("Future error collecting scan result for file '{}': {} (code: {})",
                    filePath, fe.what(), fe.code()));
            } catch (const std::exception& e) {
                failedResults++;
                logger.error(std::format("Exception collecting scan result for file '{}': {}",
                    filePath, e.what()));
            }
        }

        logger.success(std::format("Scan completed: {} files processed, {} successful, {} failed, {} with positive results",
            processedResults, successfulResults, failedResults, positiveScanResults));
        return results;
    }
} // namespace scanner