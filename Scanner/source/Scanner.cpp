#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include "support.hpp"
#include <algorithm>
#include <exception>
#include <future>
#include <iostream>
#include <unordered_map>

namespace {
    std::unordered_map<std::string, SCAN_RESULTS> scanMultipleFilesHELPER(const std::vector<std::filesystem::path> &filesPack) {
        std::unordered_map<std::string, SCAN_RESULTS> singleThreadResult;

        std::ranges::for_each(filesPack, [&singleThreadResult](const std::filesystem::path &filePath) -> void {
            SCAN_RESULTS fileResults;
            try {
                fileResults = scanner::scan_file(filePath);
            } catch (std::exception &ERROR) {
                throw ERROR;
            }
            singleThreadResult[filePath.c_str()] = fileResults;
        });

        return singleThreadResult;
    }
} // namespace

namespace scanner {
    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS {
        using namespace std::filesystem;
        SCAN_RESULTS results;

        for (const auto &directory_path : rule_engine::get_Rules()) {

            if (!exists(directory_path)) {
                std::cerr << "Directory path " << directory_path.c_str() << "do not exists!" << std::endl;
                continue;
            }

            std::vector<path> loaded_rules;
            for (const auto &rule_file : directory_iterator(directory_path)) {
                if (rule_file.is_regular_file()) {
                    loaded_rules.push_back(rule_file);
                }
            }

            std::ranges::for_each(loaded_rules, [&](const path &rule_path) -> void {
                if (exists(rule_path) && rule_path.extension() == ".yar") {
                    try {
                        YARA_Wrapper::YARA_SCAN(file_path, rule_path, &results);
                    } catch (...) {
                        std::cerr << "Error occured while scanning by rule : " << rule_path.c_str() << std::endl;
                    }
                }
            });
        }

        return results;
    }
    auto scanMultipleFiles(const std::vector<std::filesystem::path> &files, const int numberOfThreads = 4)
        -> std::unordered_map<std::string, SCAN_RESULTS> {
        auto spliited_vector = support::container_utils::split(files, numberOfThreads);
        std::vector<std::future<std::unordered_map<std::string, SCAN_RESULTS>>> futureResults;
        std::unordered_map<std::string, SCAN_RESULTS> results;

        std::ranges::for_each(spliited_vector, [&futureResults](const auto &filesPack) -> void {
            futureResults.emplace_back(std::async(std::launch::async, scanMultipleFilesHELPER, filesPack));
        });

        std::ranges::for_each(futureResults, [&results](auto &futureResults) -> void { results += futureResults.get(); });

        return results;
    }
} // namespace scanner
