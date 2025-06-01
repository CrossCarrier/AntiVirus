#include "../include/Scanner.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "../../HELPERS/include/support.hpp"
#include <exception>
#include <future>
#include <iostream>

namespace {
    SCAN_RESULTS_PACK scanMultipleFilesHELPER(const PATHS_CONTAINER &filesPack) {
        SCAN_RESULTS_PACK singleThreadResult;

        std::ranges::for_each(filesPack, [&singleThreadResult](const PATH &filePath) -> void {
            SCAN_RESULTS fileResults;

            try {

                fileResults = scanner::scan_file(filePath);

            } catch (std::exception &_) {
                throw;
            }

            singleThreadResult[filePath.c_str()] = fileResults;
        });

        return singleThreadResult;
    }
} // namespace

namespace scanner {
    auto scan_file(const PATH &file_path) -> SCAN_RESULTS {
        SCAN_RESULTS results;

        for (const auto &directory_path : rule_engine::get_Rules()) {

            if (!std::filesystem::exists(directory_path)) {
                std::cerr << "Directory path " << directory_path.c_str() << "do not exists!" << std::endl;
                continue;
            }

            PATHS_CONTAINER loaded_rules;
            for (const auto &rule_file : DIRECTORY_ITER(directory_path)) {
                if (rule_file.is_regular_file()) {
                    loaded_rules.push_back(rule_file);
                }
            }

            std::ranges::for_each(loaded_rules, [&](const PATH &rule_path) -> void {
                if (std::filesystem::exists(rule_path) && rule_path.extension() == ".yar") {
                    try {

                        yara_wrapper::YARA_SCAN(file_path, rule_path, &results);

                    } catch (...) {
                        std::cerr << "Error occured while scanning by rule : " << rule_path.c_str() << std::endl;
                    }
                }
            });
        }

        return results;
    }
    auto scanMultipleFiles(const PATHS_CONTAINER &files, int numberOfThreads)
        -> SCAN_RESULTS_PACK {

        auto spliited_vector = support::container_utils::split(files, numberOfThreads);
        std::vector<std::future<SCAN_RESULTS_PACK>> futureResults;
        SCAN_RESULTS_PACK results;

        std::ranges::for_each(spliited_vector, [&futureResults](const auto &filesPack) -> void {
            futureResults.emplace_back(std::async(std::launch::async, scanMultipleFilesHELPER, filesPack));
        });

        std::ranges::for_each(futureResults, [&results](auto &futureResults) -> void { results += futureResults.get(); });

        return results;
    }
} // namespace scanner
